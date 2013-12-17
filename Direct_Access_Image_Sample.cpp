//===========================================================================
//===========================================================================
//===========================================================================
//==   Direct_Access_Image_Sample.cpp  ==  Author: Costin-Anton BOIANGIU   ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Direct_Access_Image.h"
#include <stdlib.h>
//===========================================================================
//===========================================================================

int getMaxNeigh(BYTE **image, int y, int x, int height, int width, int radius) {
	int max = 0;

	for (int i = y - radius; i < y + radius; i++) {
		for (int j = x - radius; j < x + radius; j++) {
			if (i < 0 || i >= height || j < 0 || j >= width) break;
			if (image[i][j] > max) max = image[i][j];
		}
	}

	return max;
}

int getMinNeigh(BYTE **image, int y, int x, int height, int width, int radius) {
	int min = 256;

	for (int i = y - radius; i < y + radius; i++) {
		for (int j = x - radius; j < x + radius; j++) {
			if (i < 0 || i >= height || j < 0 || j >= width) break;
			if (image[i][j] < min) min = image[i][j];
		}
	}
 
    return min;
}

int** getBernsenThreshold(BYTE **image, int height, int width) {
	int **threshold = (int**)malloc(height * sizeof(int *));
	int **neighMax = (int**)malloc(height * sizeof(int *));
	int **neighMin = (int**)malloc(height * sizeof(int *));
	for (int i = 0; i < height; i++) {
		threshold[i] = (int*)malloc(width * sizeof(int));
		neighMax[i] = (int*)malloc(width * sizeof(int));
		neighMin[i] = (int*)malloc(width * sizeof(int));
	}

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			neighMax[i][j] = getMaxNeigh(image, i, j, height, width, 30);
			neighMin[i][j] = getMinNeigh(image, i, j, height, width, 30);
			threshold[i][j] = (neighMax[i][j] + neighMin[i][j]) / 2;
		}
	}
	return threshold;
}

//===========================================================================
//===========================================================================
int _tmain(int argc, _TCHAR* argv[])
{
    //Verify command-line usage correctness
    if (argc != 4)
    {
        _tprintf(_T("Use: %s <Input_Image_File_Name (24BPP True-Color)> <Output_Image_File_Name> <Confidence_Matrix_File_Name>\n"), argv[0]);
        return -1;
    }

    //Buffer for the new file names
    TCHAR strNewFileName[0x100];

    //Load and verify that input image is a True-Color one
    KImage *pImage = new KImage(argv[1]);
    if (pImage == NULL || !pImage->IsValid() || pImage->GetBPP() != 24)
    {
        _tprintf(_T("File %s does is not a valid True-Color image!"), argv[0]);
        return -2;
    }
	
    //Apply a Gaussian Blur with small radius to remove obvious noise
    pImage->GaussianBlur(0.5);
    _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_blurred.TIF"), argv[0]);
    pImage->SaveAs(strNewFileName, SAVE_TIFF_LZW);

    //Convert to grayscale
    KImage *pImageGrayscale = pImage->ConvertToGrayscale();
    //Don't forget to delete the original, now useless image
    delete pImage;

    //Verify conversion success...
    if (pImageGrayscale == NULL || !pImageGrayscale->IsValid() || pImageGrayscale->GetBPP() != 8)
    {
        _tprintf(_T("Conversion to grayscale was not successfull!"));
        return -3;
    }
    //... and save grayscale image
    _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_grayscale.TIF"), argv[0]);
    pImageGrayscale->SaveAs(strNewFileName, SAVE_TIFF_LZW);
    
    //Request direct access to image pixels in raw format
    BYTE **pDataMatrixGrayscale = NULL;
    if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
    {
        //If direct access is obtained get image attributes and start processing pixels
        int intWidth = pImageGrayscale->GetWidth();
        int intHeight = pImageGrayscale->GetHeight();

        //Create binary image
        KImage *pImageBinary = new KImage(intWidth, intHeight, 1);
		//Create confidence matrix
		KImage *pConfidenceImageBinary = new KImage(intWidth, intHeight, 8);
		if (pImageBinary->BeginDirectAccess() && pConfidenceImageBinary->BeginDirectAccess())
        {
            int **threshold = getBernsenThreshold(pDataMatrixGrayscale, intHeight, intWidth);
            for (int y = intHeight - 1; y >= 0; y--) {
                for (int x = intWidth - 1; x >= 0; x--)
                {
                    //You may use this instead of the line below: 
                    //    BYTE PixelAtXY = pImageGrayscale->Get8BPPPixel(x, y)
                    BYTE &PixelAtXY = pDataMatrixGrayscale[y][x];

                    if (PixelAtXY < threshold[y][x])
                        //...if closer to black, set to black
                        pImageBinary->Put1BPPPixel(x, y, false);
                    else
                        //...if closer to white, set to white
                        pImageBinary->Put1BPPPixel(x, y, true);
					pConfidenceImageBinary->Put8BPPPixel(x, y, abs(PixelAtXY - threshold[y][x]));
                }
			}
            //Close direct access
            pImageBinary->EndDirectAccess();
			pConfidenceImageBinary->EndDirectAccess();
            
            //Save binarized image
            _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), argv[2]);
            pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);
			//Save binarized confidence matrix image
			_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), argv[3]);
            pConfidenceImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

            //Don't forget to delete the binary image
            delete pImageBinary;
			delete pConfidenceImageBinary;
        }
        else
        {
            _tprintf(_T("Unable to obtain direct access in binary image!"));
            return -3;
        }

        //Close direct access
        pImageGrayscale->EndDirectAccess();
    }
    else
    {
        _tprintf(_T("Unable to obtain direct access in grayscale image!"));
        return -4;
    }

    //Don't forget to delete the grayscale image
    delete pImageGrayscale;

    //Return with success
    return 0;
}
//===========================================================================
//===========================================================================
