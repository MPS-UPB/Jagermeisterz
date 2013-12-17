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

	//Request direct access to image pixels in raw format
	BYTE **pDataMatrixGrayscale = NULL;
	if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
	{
		//If direct access is obtained get image attributes and start processing pixels
		int intWidth = pImageGrayscale->GetWidth();
		int intHeight = pImageGrayscale->GetHeight();

		//Create binary image
		KImage *pImageBinary = new KImage(intWidth, intHeight, 1);
		KImage *pConfidenceImage = new KImage(intWidth, intHeight, 8);
		if (pImageBinary->BeginDirectAccess() && pConfidenceImage->BeginDirectAccess())
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
						pConfidenceImage->Put8BPPPixel(j, i, 0x00);
					}
					else
					{
						pImageBinary->Put1BPPPixel(j, i, false);
						pConfidenceImage->Put8BPPPixel(j, i, 0xFF);
					}
				}
			}

			//Close direct access
			pImageBinary->EndDirectAccess();
			pImagePadded->EndDirectAccess();
			pConfidenceImage->EndDirectAccess();

			//Save binarized image
			_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[2]);
			pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

			_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[3]);
			pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_LZW);

			//Don't forget to delete the binary image
			delete pImageBinary;
			delete meanMatrix;
			delete accMatrix;
			delete imageMatrix;
			delete pImagePadded;
			delete pConfidenceImage;
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
