#include <stdio.h>
#include <string.h>
#define TESTING_MATH

#include "constants/load_constants.h"

#include "testing/generate_image.h"
#include "decompressor/reconstructor.h"

#if LOG
#include "files/logs.h"
#endif

char DEBUG = 0;

int main(int argInt, char **argS)
{
    #if LOG
    Log_init();
    #endif

    if (argInt > 1)
    {
        DEBUG = argInt >= 5;

        if (!strcmp(argS[1], "encode"))
        {
            EncodeImage(argS[2], argS[3]);
        }
        else if (!strcmp(argS[1], "predict"))
        {
            PredictImage(argS[2], argS[3]);
        }
        else if (!strcmp(argS[1], "compress"))
        {
            CompressImage(argS[2], argS[3]);
        }
        else
        {
            printf("Command Not Recognized\n");
        }
    }
    else
    {
        printf("Running Base Case.\n");
        // Base Case if no command line aguments are given.
        PredictImage("../data/test-images/indian_pines.csv", "../data/results/CRESULT_indian_pines_predicted.csv");
        //TestReconstructor("../data/test-images/TEST_100x100x100_0.csv");
        // EncodeImage("../data/results/CRESULT_100x100x100_0_predicted.csv", "../data/results/CRESULT_100x100x100_0_encoded.hsp");
    }

    #if LOG
    Log_close();
    #endif

    return 0;
}