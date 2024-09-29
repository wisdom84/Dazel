
#include "Math_test_file.h"
#include "core/logger.h"
void print_matrix(mat4 matrix_0, int num){
    for (auto i = 0; i < num; i++)
		{
			for (auto j = 0; j < num; j++)
			{
				DDEBUG("row %i : %2f",i, matrix_0.elements[i][j]);
			}
         DDEBUG("=======================================================");
	}
}
void print_matrix_3(mat3 matrix_0, int num){
        for (auto i = 0; i < num; i++)
		{
			for (auto j = 0; j < num; j++)
			{
				DDEBUG("row %i : %2f",i, matrix_0.elements[i][j]);
			}
         DDEBUG("=======================================================");
	}
}