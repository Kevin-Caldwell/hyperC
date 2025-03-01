#include "decompressor/reconstructor.h"

image *sample;
void Reconstructor(image *predicted_values, image *reconstructed, FILE *file_ptr)
{
    LoadConstantFile(PREDICTOR_CONSTANTS_LOCATION, &predictor_constants);
    InitalizePredictorConstants();
    time_t start;
    time_t end;
    dim3 size = predicted_values->size;

    weight_t weights[kP];

    start = time(NULL);
    for (int z = 0; z < size.z; z++)
    {
        if (!kPredictionMode)
        {
            kC = Ps(z) + 3;
        }
        else
        {
            kC = Ps(z);
        }
        for (int y = 0; y < size.y; y++)
        {
            for (int x = 0; x < size.x; x++)
            {
                ReconstructPixel(predicted_values, reconstructed, z, y, x, file_ptr, weights);
            }
        }

        time_t time_elapsed = time(NULL) - start;
        time_t time_left = time_elapsed * (predicted_values->size.z - z - 1) / (z + 1);

        #ifdef ARM_COMPILE
        printf("\rPredicted %d/%d of Image. (%lld seconds Elapsed, %lld seconds Left)", (int)(z + 1), (int)size.z, time_elapsed, time_left);
        #else 
        printf("\rPredicted %d/%d of Image. (%ld seconds Elapsed, %ld seconds Left)", (int)(z + 1), (int)size.z, time_elapsed, time_left);
        #endif

        fflush(stdout);
    }
    end = time(NULL);
    printf("\n%d seconds for image prediction.\n", (int)(end - start));
}

void ReconstructPixel(image *mapped, image *data, INDEX z, INDEX y, INDEX x, FILE *file_ptr, weight_t* weights)
{

    data_t mapped_data = GetPixel(mapped, x, y, z);
    uint16_t local_sum = L_FindLocalSum(data, z, y, x);

    int64_t predicted_central_local_difference = L_PredictedCentralLocalDifference(data, z, y, x, weights);

    int64_t high_resolution_predicted_sample = HighResolutionPredictedSample(predicted_central_local_difference, local_sum);
    int32_t double_resolution_predicted_sample = L_DoubleResolutionPredictedSample(data, z, y, x, high_resolution_predicted_sample);

    uint16_t predicted_sample = PredictedSample(double_resolution_predicted_sample);

    int32_t delta = QuantizerIndexUnmapper(mapped_data,
                                           predicted_sample,
                                           double_resolution_predicted_sample);
    uint16_t clipped_quantizer_bin_center = ClippedQuantizerBinCenter(predicted_sample + delta);
    int32_t double_resolution_predicted_error = DoubleResolutionPredictionError(clipped_quantizer_bin_center,
                                                                                double_resolution_predicted_sample);
    // int32_t double_resolution_predicted_error = 2 * (predicted_sample + delta) - double_resolution_predicted_error
    SetPixel(data, x, y, z, (uint16_t)predicted_sample + delta);

    L_UpdateWeights(data, weights, z, y, x, double_resolution_predicted_error);

    char write_buffer[1000];

    #ifdef ARM_COMPILE
    sprintf(write_buffer, "(%ld,%ld,%ld),%lu, %d, %d, %lld, %ld, %lld, [", x, y, z, delta + predicted_sample, predicted_sample, mapped_data,
            predicted_central_local_difference, double_resolution_predicted_sample, high_resolution_predicted_sample);
    #else 
    sprintf(write_buffer, "(%d,%d,%d),%u, %d, %d, %ld, %d, %ld, [", x, y, z, delta + predicted_sample, predicted_sample, mapped_data,
        predicted_central_local_difference, double_resolution_predicted_sample, high_resolution_predicted_sample);
    #endif

    for (int i = 0; i < kC; i++)
    {
        #ifdef ARM_COMPILE
        sprintf(write_buffer + strlen(write_buffer), "%ld,", weights[i]);
        #else 
        sprintf(write_buffer + strlen(write_buffer), "%d,", weights[i]);
        #endif
    }
    sprintf(write_buffer + strlen(write_buffer), "]\n");

    #ifdef ARM_COMPILE
    sprintf(write_buffer + strlen(write_buffer), "UpdateWeights args: %ld %ld %ld %ld\n", z, y, x, double_resolution_predicted_error);
    #else
    sprintf(write_buffer + strlen(write_buffer), "UpdateWeights args: %d %d %d %d\n", z, y, x, double_resolution_predicted_error);
    #endif

    F_WRITE(write_buffer, sizeof(char), strlen(write_buffer), file_ptr);
    
}

void TestReconstructor(char *file_name)
{
    file_t *file_ptr;

    printf("Testing Reconstructor...\n");
    image sample_data;
    ReadImageFromCSV(&sample_data, file_name);
    printf("Finished Reading Image.\n");
    sample = &sample_data;

    image predicted_data;
    InitImage(&predicted_data, sample_data.size.x, sample_data.size.y, sample_data.size.z);

    printf("Running Predictor...\n");
    fflush(stdout);
    RunPredictor(&sample_data, &predicted_data);
    printf("Completed Prediction.\n");

    printf("Logging to logs/c-debug.LOG.\n");
    file_ptr = F_OPEN("../data/logs/c-reconstructor-debug.LOG", WRITE);

    image reconstructed_data;
    InitImage(&reconstructed_data, sample_data.size.x, sample_data.size.y, sample_data.size.z);

    printf("Running Reconstructor...\n");
    Reconstructor(&predicted_data, &reconstructed_data, file_ptr);
    printf("Completed Reconstruction.\n");

    int res = Image_Equals(&sample_data, &reconstructed_data);

    if (res == INT32_MAX)
    {
        printf("Reconstruction Success!\n");
    }
    else
    {
        printf("Reconstruction Failed at i= %d.\n", res);
    }

    F_CLOSE(file_ptr);
}