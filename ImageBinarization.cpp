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
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
int _tmain(int argc, _TCHAR* argv[])
{
    //Verify command-line usage correctness
    if (argc != 4)
    {
        _tprintf(_T("Use: %s <Input_Image_File_Name > <Output_Image_File_Name> <Confidence_Image_Name\n"), argv[0]);
        return -1;
    }

    //Buffer for the new file names
    TCHAR strNewFileName[0x100];

    //Load and verify that input image is valid
    KImage *pImage = new KImage(argv[1]);
	if (pImage == NULL || !pImage->IsValid() || (pImage->GetBPP() != 24 && pImage->GetBPP() != 8))
    {
        _tprintf(_T("File %s does is not a valid image!"), argv[0]);
        return -2;
    }


    //Apply a Gaussian Blur with small radius to remove obvious noise
    pImage->GaussianBlur(0.5);
    //_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[1]);
    //pImage->SaveAs(strNewFileName, SAVE_TIFF_LZW);

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
    //_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[1]);
    //pImageGrayscale->SaveAs(strNewFileName, SAVE_TIFF_LZW);
    
    //Request direct access to image pixels in raw format
    BYTE **pDataMatrixGrayscale = NULL;
    if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
    {
        //If direct access is obtained get image attributes and start processing pixels
        int intWidth = pImageGrayscale->GetWidth();
        int intHeight = pImageGrayscale->GetHeight();

		//Generate image histogram
		pImageGrayscale->initImageHistogram();
		pImageGrayscale->createImageHistogram();

        //Create binary image
        KImage *pImageBinary = new KImage(intWidth, intHeight, 1);

		//Create confidence image
		KImage *pImageConfidence = new KImage(intWidth, intHeight, 8);
		if (pImageBinary->BeginDirectAccess())
        {
			if(pImageConfidence->BeginDirectAccess()){
				int newPixel;
				std::vector<int> thresholds = pImageGrayscale->getBinarizationThreshold();
				
				for(int k = 0; k < 9; k++){
					for (int y = (k+1)*pImageGrayscale->tileHeight - 1; y >= k*pImageGrayscale->tileHeight; y--){
						for (int x = intWidth - 1; x >= 0; x--){
							BYTE PixelAtXY = pImageGrayscale->Get8BPPPixel(x, y);
							if(PixelAtXY >= thresholds[k]){
								pImageBinary->Put1BPPPixel(x, y, true);
								pImageConfidence->Put8BPPPixel(x, y, PixelAtXY-thresholds[k]);
							}
							else{
								pImageBinary->Put1BPPPixel(x, y, false);
								pImageConfidence->Put8BPPPixel(x, y, thresholds[k]-PixelAtXY);
							}
						}
					}
				}

				for (int y = intHeight - 1; y >= 9*pImageGrayscale->tileHeight; y--){
					for (int x = intWidth - 1; x >= 0; x--){
						BYTE PixelAtXY = pImageGrayscale->Get8BPPPixel(x, y);
						if(PixelAtXY >= thresholds[9]){
							pImageBinary->Put1BPPPixel(x, y, true);
							pImageConfidence->Put8BPPPixel(x, y, PixelAtXY-thresholds[9]);
						}
						else{
							pImageBinary->Put1BPPPixel(x, y, false);
							pImageConfidence->Put8BPPPixel(x, y, thresholds[9]-PixelAtXY);
						}
					}
				}
				
				//Close direct access
				pImageBinary->EndDirectAccess();
				pImageConfidence->EndDirectAccess();
            
				//Save binarized image
				_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[2]);
				pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

				//Save confidence image
				_stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[3]);
				pImageConfidence->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

				//Delete the binary image and confidence image
				delete pImageBinary;
				delete pImageConfidence;
			}
			else{
				_tprintf(_T("Unable to obtain direct access in confidence image!"));
				return -3;
			}
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
