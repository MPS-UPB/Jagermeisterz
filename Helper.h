#include <math.h>

#define SAUVOLA_M		3
#define SAUVOLA_N		3

void CalculateDifference(double** matrix1, double** matrix2, int intWidth, int intHeight)
{
	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			matrix1[i][j] -= matrix2[i][j];
		}
	}
}

void CalcuateSqaureRoot(double** matrix, int intWidth, int intHeight)
{
	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			matrix[i][j] = sqrt(matrix[i][j]);
		}
	}
}

double CalculateMaximum(double** matrix, int intWidth, int intHeight)
{
	double max = -1.0;

	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			if (matrix[i][j] > max)
			{
				max = matrix[i][j];
			}
		}
	}

	return max;
}

double** SquareMatrix(double** matrix, int intHeight, int intWidth)
{
	double** squareMatrix = new double*[intHeight];
	for (int i = 0; i < intHeight; i++)
	{
		squareMatrix[i] = new double[intWidth];
	}

	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			squareMatrix[i][j] = matrix[i][j] * matrix[i][j];
		}
	}

	return squareMatrix;
}

double** CalculateDeviation(double** meanMatrix, double** meanSquareMatrix, int intWidth, int intHeight)
{
	double** meanMatrixSquare = SquareMatrix(meanMatrix, intHeight, intWidth);
	CalculateDifference(meanSquareMatrix, meanMatrixSquare, intWidth, intHeight);
	CalcuateSqaureRoot(meanSquareMatrix, intWidth, intHeight);

	return meanSquareMatrix;
}

void MultiplyMatrix(double** matrix, double k, int intWidth, int intHeight)
{
	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			matrix[i][j] *= k;
		}
	}
}

void Add(double** matrix, double k, int intWidth, int intHeight)
{
	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			matrix[i][j] += k;
		}
	}
}

void Multiply2Matrices(double** matrix1, double** matrix2, int intWidth, int intHeight)
{
	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			matrix1[i][j] *= matrix2[i][j];
		}
	}
}

double** CalculateThreshold(double** meanMatrix, double** deviationMatrix, double r, double k, int intHeight, int intWidth)
{
	MultiplyMatrix(deviationMatrix, 1 / r, intWidth, intHeight);
	Add(deviationMatrix, -1, intWidth, intHeight);
	MultiplyMatrix(deviationMatrix, k, intWidth, intHeight);
	Add(deviationMatrix, 1, intWidth, intHeight);
	Multiply2Matrices(meanMatrix, deviationMatrix, intWidth, intHeight);
	return meanMatrix;
}

double** CalculateMeanMatrix(double** accPaddedMatrix, int intWidth, int intHeight)
{
	double** meanMatrix = new double*[intHeight];
	for (int i = 0; i < intHeight; i++)
	{
		meanMatrix[i] = new double[intWidth];
	}

	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 0; j < intWidth; j++)
		{
			meanMatrix[i][j] = (accPaddedMatrix[SAUVOLA_N + i][SAUVOLA_M + j] + accPaddedMatrix[i][j] - accPaddedMatrix[SAUVOLA_N + i][j]
			- accPaddedMatrix[i][SAUVOLA_M + j]) / (SAUVOLA_M * SAUVOLA_N);
		}
	}

	return meanMatrix;
}

double** AccumulateMatrix(double** matrix, int height, int width)
{
	int intHeight = height + SAUVOLA_N;
	int intWidth = width + SAUVOLA_M;

	//allocate new double matrix
	double** accPaddedMatrix = new double*[intHeight];
	for (int i = 0; i < intHeight; i++)
	{
		accPaddedMatrix[i] = new double[intWidth];
	}

	//accumulated sum on lines
	for (int i = 0; i < intHeight; i++)
	{
		accPaddedMatrix[i][0] = matrix[i][0];
	}
	for (int i = 0; i < intHeight; i++)
	{
		for (int j = 1; j < intWidth; j++)
		{
			accPaddedMatrix[i][j] = accPaddedMatrix[i][j - 1] + matrix[i][j];
		}
	}

	//accumulated sum on columns
	for (int j = 0; j < intWidth; j++)
	{
		for (int i = 1; i < intHeight; i++)
		{
			accPaddedMatrix[i][j] = accPaddedMatrix[i - 1][j] + accPaddedMatrix[i][j];
		}
	}

	return accPaddedMatrix;
}