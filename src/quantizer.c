#include "predictor/quantizer.h"

uint16_t ClippedQuantizerBinCenter(uint16_t sample_value){
    return  sample_value;
}

int16_t PredictionResidual(uint16_t sample_value, uint16_t predicted_sample){
    return sample_value - predicted_sample;
}

int32_t QuantizerIndex(uint16_t sample_value, uint16_t predicted_sample){
    return PredictionResidual(sample_value, predicted_sample);

    // SignedLongData qInd;
    // uint16_t pred_res = PredictionResidual(hIMG, z,y,x);
    // if(x == 0 && y == 0){
    //     qInd = pred_res;
    // }
    // else{
    //     qInd = SIGN(pred_res) * (abs(pred_res) + m_z) / (2 * m_z + 1);
    // }
    // return qInd;

}


uint16_t minTheta(uint32_t predicted_sample){

    // if(x == 0 && y == 0){
    //     return MIN(pred_res - kSmin, kSmax - pred_res);
    // } else if (x > 0 || y > 0)
    // {
    //     return  MIN((pred_res - kSmin + m_z)/(2 * m_z + 1), 
    //             (kSmax - pred_res + m_z)/(2 * m_z + 1));
    // }
    return MIN(predicted_sample - kSmin, kSmax - predicted_sample);
    
}


data_t MappedQuantizerIndex(int32_t quantizer_index, 
            int32_t predicted_sample, 
            uint32_t double_res_predicted_sample){

    int32_t theta = minTheta(predicted_sample);
    int32_t a = quantizer_index * (double_res_predicted_sample % 2 ? -1 : 1);

    if(abs(quantizer_index) > theta){
        return abs(quantizer_index) + theta;
    } else if (0 <= a && a <= theta)
    {
        return 2 * abs(quantizer_index);
    } else{
        return 2 * abs(quantizer_index) - 1;
    }
    
}