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
#include "Helper.h"
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================

int _tmain(int argc, _TCHAR* argv[])
{
    //Verify command-line usage correctness
    if (argc != 2)
    {
        _tprintf(_T("Use: %s <Input_Image_File_Name (24BPP True-Color)>\n"), argv[0]);
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
        if (pImageBinary->BeginDirectAccess())
        {
			KImage *pImagePadded = pImageGrayscale->PadImage();
			double **imageMatrix = pImagePadded->ConvertToDouble();
			double **accMatrix = AccumulateMatrix(imageMatrix, intHeight, intWidth);
			double **meanMatrix = CalculateMeanMatrix(accMatrix, intWidth, intHeight);

			double **squareMatrix = SquareMatrix(imageMatrix, intHeight + SAUVOLA_N, intWidth + SAUVOLA_M);
			double **accSquareMatrix = AccumulateMatrix(squareMatrix,intHeight, intWidth);
			double **meanSquareMatrix = CalculateMeanMatrix(accSquareMatrix, intWidth, intHeight);

			double **deviationMatrix = CalculateDeviation(meanMatrix, meanSquareMatrix, intWidth, intHeight);
			double R = CalculateMaximum(deviationMatrix, intWidth, intHeight);
			double **thresholdMatrix = CalculateThreshold(meanMatrix, deviationMatrix, R, K, intHeight, intWidth);

			for (int i = 0; i < intHeight; i++)
			{
				for (int j = 0; j < intWidth; j++)
				{
					if ((double)pImageGrayscale->Get8BPPPixel(j, i) > thresholdMatrix[i][j])
					{
						pImageBinary->Put1BPPPixel(j, i, true);
					}
					else
					{
						pImageBinary->Put1BPPPixel(j, i, false);
					}
				}
			}

            //Close direct access
            pImageBinary->EndDirectAccess();
			pImagePadded->EndDirectAccess();
            
            //Save binarized image
            _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_Black_and_White.TIF"), argv[0]);
            pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

            //Don't forget to delete the binary image
            delete pImageBinary;
			delete meanMatrix;
			delete accMatrix;
			delete imageMatrix;
			delete pImagePadded;
			delete squareMatrix;
			delete accSquareMatrix;
			delete meanSquareMatrix;
			delete deviationMatrix;
			delete thresholdMatrix;
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
