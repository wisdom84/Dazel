#pragma once

#include <time.h>

#include <math.h>

#include <cstdlib>

#include <stdlib.h>
#include <string.h>

#define FLOAT_MAX 3459.877

#define PI 3.14159265358979323846f

#define PI_2 PI * 2.0f

#define PI_HALF PI * 0.5f

#define PI_QUARTER PI * 0.25f

#define ONE_OVER_PI 1.0f / PI

#define ONE_OVER_PI_2 1.0f / PI_2

#define SQRT_2 1.41421356237309504880f

#define SQRT_3 1.73205080756887729352f

#define SQRT_ONE_OVER_TWO 0.70710678118654752440f

#define SQRT_ONE_OVER_THREE 0.57735026918962576450f

#define DEGRAD_MUTIPLE PI / 180.f

#define RADDEG_MULTIPLE 180 / PI

// the  multiplier to convert seconds to miliseconds
#define SEC_MILSEC 1000.0f;

#define MILSEC_SEC 0.001f;

#define EPSILON 1.92092896e-07f

inline float dsin(float x)
{
	return (float)(sin(x * DEGRAD_MUTIPLE));
};

inline float dcos(float x)
{
	return (float)(cos(x * DEGRAD_MUTIPLE));
};

inline float dtan(float x)
{
	return (float)(tan(x * DEGRAD_MUTIPLE));
};

inline float dcot(float x)
{
	return (float)(cos(x * DEGRAD_MUTIPLE) / sin(x * DEGRAD_MUTIPLE));
};

inline float dasin(float x)
{
	return (float)(asin(x) * RADDEG_MULTIPLE);
};

inline float dacos(float x)
{
	return (float)(acos(x) * RADDEG_MULTIPLE);
};

inline float datan(float x)
{
	return (float)(atan(x) * RADDEG_MULTIPLE);
};

inline float dacot(float x)
{
	return (float)(atan(1.0f / x) * RADDEG_MULTIPLE);
};

inline float dabs(float x)
{
	return (float)(abs(x));
};

inline float dsqrt(float x)
{
	return (float)(sqrt(x));
};

inline bool is_power_of_two(int value)
{
	return ((value != 0) && ((value & (value - 1)) == 0));
}

inline bool seedrand = false;

inline int fish()
{
	if (!seedrand)
	{
		srand(time(0));
		seedrand = true;
	}
	return (int)rand();
}

inline int RandomIntRange(int min, int max)
{
	if (!seedrand)
	{
		srand(time(0));
		seedrand = true;
	}
	return (int)((rand() % max - min + 1) + min);
};

inline float RandomFloat()
{
	if (!seedrand)
	{
		srand(time(0));
		seedrand = true;
	}
	return ((((float)rand()) / RAND_MAX) * FLOAT_MAX);
};

inline float RandomFloatRange(float min, float max)
{
	if (!seedrand)
	{
		srand(time(0));
		seedrand = true;
	}
	return ((((float)rand() / RAND_MAX) * (max - min)) + min);
};

namespace dlm
{

	typedef union vec2_u
	{
		float elements[2];
		struct
		{
			union
			{
				float x, r, s, u;
			};
			union
			{
				float y, g, t, v;
			};
		};
	} vec2;

	typedef union ve3_u
	{
		float elements[3];
		struct
		{
			union
			{
				float x, r, s, u;
			};
			union
			{
				float y, g, t, v;
			};
			union
			{
				float z, b, p, w;
			};
		};
	} vec3;

	typedef union vec4_u
	{
		float elements[4];
		struct
		{
			union
			{
				float x, r, s;
			};
			union
			{
				float y, g, t;
			};
			union
			{
				float z, b, p;
			};
			union
			{
				float w, a, q;
			};
		};
	} vec4;
	typedef union mat2_u
	{

		float elements[2][2];

		struct
		{
			union
			{
				vec2 a1;
			};
			union
			{
				vec2 a2;
			};
		};
	} mat2;
	typedef union mat3_u
	{

		float elements[3][3];

		struct
		{
			union
			{
				vec3 a1;
			};
			union
			{
				vec3 a2;
			};
			union
			{
				vec3 a3;
			};
		};
	} mat3;

	typedef union mat4_u
	{
		float elements[4][4];
		struct
		{
			union
			{
				vec4 a1;
			};
			union
			{
				vec4 a2;
			};
			union
			{
				vec4 a3;
			};
			union
			{
				vec4 a4;
			};
		};
	} mat4;

	typedef struct Vertex_3d{
        vec3 position;
		vec2 texture_cord;  
	}Vertex_3d;
    
	typedef struct Vertex_2d{
        vec2 position;
		vec2 texture_cord;  
	}Vertex_2d;
	//======vec2 class================
	inline vec2 vec2_create(float x, float y)
	{
		vec2 out_vec2;
		out_vec2.x = x;
		out_vec2.y = y;
		return out_vec2;
	}
	inline vec2 Add_vec2(vec2 vector_0, vec2 vector_1)
	{
		vec2 out_vec2;
		out_vec2.x = vector_0.x + vector_1.x;
		out_vec2.y = vector_0.y + vector_1.y;
		return out_vec2;
	}
	inline vec2 Subtraction_vec2(vec2 vector_0, vec2 vector_1)
	{
		vec2 out_vec2;
		out_vec2.x = vector_1.x - vector_0.x;
		out_vec2.y = vector_1.y - vector_0.y;
		return out_vec2;
	}
	inline vec2 Division_vec2(vec2 vector_0, float number)
	{
		vec2 out_vec2;
		out_vec2.x = vector_0.x / number;
		out_vec2.y = vector_0.y / number;
		return out_vec2;
	}
	inline float Dot_prod_vec2(vec2 vector_0, vec2 vector_1)
	{
		return ((vector_0.x * vector_1.x) + (vector_0.y * vector_1.y));
	}
	inline vec2 vec2_zero()
	{
		return vec2{0.0f, 0.0f};
	}
	inline vec2 vec2_one()
	{
		return vec2{1.0f, 1.0f};
	}
	inline vec2 vec2_up()
	{
		return vec2{0.0f, 1.0f};
	}
	inline vec2 vec2_down()
	{
		return vec2{0.0f, -1.0f};
	}
	inline vec2 vec2_left()
	{
		return vec2{-1.0f, 0.0f};
	}
	inline vec2 vec2_right()
	{
		return vec2{1.0f, 0.0f};
	}
	inline float vec2_length(vec2 vector_0)
	{
		return (dsqrt((vector_0.x * vector_0.x) + (vector_0.y * vector_0.y)));
	}
	inline vec2 vec2_nomalize(vec2 vector_0)
	{
		vec2 out_vec2;
		out_vec2.x = vector_0.x / vec2_length(vector_0);
		out_vec2.y = vector_0.y / vec2_length(vector_0);
		return out_vec2;
	}
	inline float vec2_angle(vec2 vector_0, vec2 vector_1)
	{
		return (dacos(Dot_prod_vec2(vector_0, vector_1) / (vec2_length(vector_0) * vec2_length(vector_1))));
	}
	inline vec2 vec2_scale(vec2 vector_0, float number)
	{
		vec2 out_vec2;
		out_vec2.x = vector_0.x * number;
		out_vec2.y = vector_0.y * number;
		return out_vec2;
	}
	inline vec2 vec2_copy(vec2 vector_0)
	{
		vec2 out_vec2;
		out_vec2.x = vector_0.x;
		out_vec2.y = vector_0.y;
		return out_vec2;
	}
	inline float vec2_distance(vec2 vector_0, vec2 vector_1)
	{
		return (vec2_length(Subtraction_vec2(vector_0, vector_1)));
	}
	inline float vec2_projection(vec2 vector_0, vec2 vector_1)
	{
		return (Dot_prod_vec2(vector_0, vector_1) / vec2_length(vector_1));
	}
	inline vec3 vec2_to_vec3_homogenous(vec2 vector_0)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x;
		out_vec3.y = vector_0.y;
		out_vec3.z = 1.0f;
		return out_vec3;
	}
	inline vec2 vec3_to_vec2(vec3 vector_0)
	{
		vec2 out_vec2;
		out_vec2.x = vector_0.x;
		out_vec2.y = vector_0.y;
		return out_vec2;
	}
	inline vec3 vec3_create(float x, float y, float z)
	{
		vec3 out_vec3;
		out_vec3.x = x;
		out_vec3.y = y;
		out_vec3.z = z;
		return out_vec3;
	}
	inline vec3 Add_vec3(vec3 vector_0, vec3 vector_1)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x + vector_1.x;
		out_vec3.y = vector_0.y + vector_1.y;
		out_vec3.z = vector_0.z + vector_1.z;
		return out_vec3;
	}
	inline vec3 Subtraction_vec3(vec3 vector_0, vec3 vector_1)
	{
		vec3 out_vec3;
		out_vec3.x = vector_1.x - vector_0.x;
		out_vec3.y = vector_1.y - vector_0.y;
		out_vec3.z = vector_1.z - vector_0.z;
		return out_vec3;
	}
	inline vec3 &operator-(vec3 vector_0, vec3 vector_1)
	{
		vec3 out_vec3;
		out_vec3.x = vector_1.x - vector_0.x;
		out_vec3.y = vector_1.y - vector_0.y;
		out_vec3.z = vector_1.z - vector_0.z;
		return out_vec3;
	}

	inline vec3 Division_vec3(vec3 vector_0, float number)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x / number;
		out_vec3.y = vector_0.y / number;
		out_vec3.z = vector_0.z / number;
		return out_vec3;
	}
	inline float Dot_prod_vec3(vec3 vector_0, vec3 vector_1)
	{
		return ((vector_0.x * vector_1.x) + (vector_0.y * vector_1.y) + (vector_0.z * vector_1.z));
	}
	inline vec3 Cross_prod_vec3(vec3 vector_0, vec3 vector_1)
	{
		vec3 out_vec3;
		out_vec3.x = (vector_0.y * vector_1.z) - (vector_0.z * vector_1.y);
		out_vec3.y = -((vector_0.x * vector_1.z) - (vector_0.z * vector_1.x));
		out_vec3.z = (vector_0.x * vector_1.y) - (vector_0.y * vector_1.x);
		return out_vec3;
	}
	inline float vec3_length(vec3 vector_0)
	{
		return (dsqrt((vector_0.x * vector_0.x) + (vector_0.y * vector_0.y) + (vector_0.z * vector_0.z)));
	}
	inline vec3 vec3_nomalize(vec3 vector_0)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x / vec3_length(vector_0);
		out_vec3.y = vector_0.y / vec3_length(vector_0);
		out_vec3.z = vector_0.z / vec3_length(vector_0);
		return out_vec3;
	}
	inline float vec3_angle(vec3 vector_0, vec3 vector_1)
	{
		return (dacos(Dot_prod_vec3(vector_0, vector_1) / (vec3_length(vector_0) * vec3_length(vector_1))));
	}
	inline vec3 vec3_scale(vec3 vector_0, float number)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x * number;
		out_vec3.y = vector_0.y * number;
		out_vec3.z = vector_0.z * number;
		return out_vec3;
	}
	inline vec3 vec3_copy(vec3 vector_0)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x;
		out_vec3.y = vector_0.y;
		out_vec3.z = vector_0.z;
		return out_vec3;
	}
	inline float vec3_distance(vec3 vector_0, vec3 vector_1)
	{
		return (vec3_length(Subtraction_vec3(vector_0, vector_1)));
	}

	inline float vec3_projection(vec3 vector_0, vec3 vector_1)
	{
		return (Dot_prod_vec3(vector_0, vector_1) / vec3_length(vector_1));
	}
	inline vec3 vec3_zero()
	{
		return vec3{0.0f, 0.0f, 0.0f};
	}
	inline bool vec3_compare(vec3 vector_0,vec3 vector_1, float tolerance){
        if((dabs(vector_0.x -vector_1.x) > tolerance || dabs(vector_0.y -vector_1.y) > tolerance)
		|| dabs(vector_0.z -vector_1.z) > tolerance){
			return false;
		}
		return true;
	}
	inline vec4 vec3_to_vec4_homogenous(vec3 vector_0)
	{
		vec4 out_vec4;
		out_vec4.x = vector_0.x;
		out_vec4.y = vector_0.y;
		out_vec4.z = vector_0.z;
		out_vec4.w = 1.0f;
		return out_vec4;
	}
	inline vec3 vec4_to_vec3(vec4 vector_0)
	{
		vec3 out_vec3;
		out_vec3.x = vector_0.x;
		out_vec3.y = vector_0.y;
		out_vec3.z = vector_0.z;
		return out_vec3;
	}

	//======vec4 class================
	inline vec4 vec4_create(float x, float y, float z, float w)
	{
		vec4 out_vec4;
		out_vec4.x = x;
		out_vec4.y = y;
		out_vec4.z = z;
		out_vec4.w = w;
		return out_vec4;
	}
	inline vec4 Add_vec4(vec4 vector_0, vec4 vector_1)
	{
		vec4 out_vec4;
		out_vec4.x = vector_0.x + vector_1.x;
		out_vec4.y = vector_0.y + vector_1.y;
		out_vec4.z = vector_0.z + vector_1.z;
		out_vec4.w = vector_0.w + vector_1.w;
		return out_vec4;
	}
	inline vec4 Subtraction_vec4(vec4 vector_0, vec4 vector_1)
	{
		vec4 out_vec4;
		out_vec4.x = vector_1.x - vector_0.x;
		out_vec4.y = vector_1.y - vector_0.y;
		out_vec4.z = vector_1.z - vector_0.z;
		out_vec4.w = vector_1.w - vector_0.w;
		return out_vec4;
	}
	inline vec4 Division_vec4(vec4 vector_0, float number)
	{
		vec4 out_vec4;
		out_vec4.x = vector_0.x / number;
		out_vec4.y = vector_0.y / number;
		out_vec4.z = vector_0.z / number;
		out_vec4.w = vector_0.w / number;
		return out_vec4;
	}
	inline float Dot_prod_vec4(vec4 vector_0, vec4 vector_1)
	{
		return ((vector_0.x * vector_1.x) + (vector_0.y * vector_1.y) + (vector_0.z * vector_1.z) + (vector_0.w * vector_1.w));
	}
	inline float vec4_length(vec4 vector_0)
	{
		return (dsqrt((vector_0.x * vector_0.x) + (vector_0.y * vector_0.y) + (vector_0.z * vector_0.z) + (vector_0.w * vector_0.w)));
	}
	inline vec4 vec4_nomalize(vec4 vector_0)
	{
		vec4 out_vec4;
		out_vec4.x = vector_0.x / vec4_length(vector_0);
		out_vec4.y = vector_0.y / vec4_length(vector_0);
		out_vec4.z = vector_0.z / vec4_length(vector_0);
		out_vec4.w = vector_0.w / vec4_length(vector_0);
		return out_vec4;
	}

	inline vec4 vec4_scale(vec4 vector_0, float number)
	{
		vec4 out_vec4;
		out_vec4.x = vector_0.x * number;
		out_vec4.y = vector_0.y * number;
		out_vec4.z = vector_0.z * number;
		out_vec4.w = vector_0.w * number;
		return out_vec4;
	}
	inline vec4 vec4_copy(vec4 vector_0)
	{
		vec4 out_vec4;
		out_vec4.x = vector_0.x;
		out_vec4.y = vector_0.y;
		out_vec4.z = vector_0.z;
		out_vec4.w = vector_0.w;
		return out_vec4;
	}
	inline float vec4_distance(vec4 vector_0, vec4 vector_1)
	{
		return (vec4_length(Subtraction_vec4(vector_0, vector_1)));
	}
	inline vec4 vec4_zero()
	{
		return vec4{0.0f, 0.0f, 0.0f, 0.0f};
	}
	inline bool vec4_compare(vec4 vector_0, vec4 vector_1, float tolerance){
			 for(auto i=0; i < 4; i++){
				if(vector_0.elements[i] != vector_1.elements[i]){
					return false;
				}
			 }
		
	  return true;		 
	}
	////=============================matrix===============================
	inline mat3 mat3_zero()
	{
		mat3 out_mat3;
		out_mat3.a1 = vec3_copy(vec3_zero());
		out_mat3.a2 = vec3_copy(vec3_zero());
		out_mat3.a3 = vec3_copy(vec3_zero());
		return out_mat3;
	};
	inline mat3 mat3_identity()
	{
		mat3 out_mat3 = mat3_zero();
		out_mat3.a1.elements[0] = 1.0f;
		out_mat3.a2.elements[1] = 1.0f;
		out_mat3.a3.elements[2] = 1.0f;
		return out_mat3;
	}
	inline mat3 mat3_homogenous_translation(float x, float y)
	{
		mat3 out_mat3_h = mat3_identity();
		out_mat3_h.a1.elements[2] = x;
		out_mat3_h.a2.elements[2] = y;
		return out_mat3_h;
	}
	inline mat3 mat3_homogenous_rotation(float angle)
	{
		mat2 out_mat2;
		vec3 out_vec3;
		out_mat2.a1 = vec2_create(dcos(angle), -dsin(angle));
		out_mat2.a2 = vec2_create(dsin(angle), dcos(angle));
		mat3 matrix = mat3_identity();
		matrix.a1.elements[0] = out_mat2.a1.elements[0];
		matrix.a1.elements[1] = out_mat2.a1.elements[1];
		matrix.a2.elements[0] = out_mat2.a2.elements[0];
		matrix.a2.elements[1] = out_mat2.a2.elements[1];
		return matrix;
	}
	inline mat3 mat3_homogenous_scale(float x, float y, float z)
	{
		mat3 out_mat3 = mat3_identity();
		out_mat3.a1.elements[0] = x;
		out_mat3.a2.elements[1] = y;
		out_mat3.a3.elements[2] = z;
		return out_mat3;
	}

	//	//==============================mat2================================

	inline mat2 mat2_zero()
	{
		mat2 out_mat2;
		out_mat2.a1 = vec2_copy(vec2_zero());
		out_mat2.a2 = vec2_copy(vec2_zero());
		return out_mat2;
	}
	inline mat2 mat2_identity()
	{
		mat2 out_mat2 = mat2_zero();
		out_mat2.a1.elements[0] = 1.0f;
		out_mat2.a2.elements[1] = 1.0f;
		return out_mat2;
	}
	inline mat2 mat2_multiply(mat2 matrix_0, mat2 matrix_1)
	{
		mat2 out_mat2;
		for (auto i = 0; i < 2; i++)
		{
			for (auto j = 0; j < 2; j++)
			{
				float value = 0.0;
				for (auto k = 0; k < 2; k++)
				{
					value += matrix_0.elements[i][k] * matrix_1.elements[k][j];
				}
				out_mat2.elements[i][j] = value;
			}
		}
		return out_mat2;
	}
	inline mat2 mat2_scale(mat2 matrix_0, float factor)
	{
		for (auto i = 0; i < 2; i++)
		{
			for (auto j = 0; j < 2; j++)
			{
				matrix_0.elements[i][j] = matrix_0.elements[i][j] * factor;
			}
		}
		return matrix_0;
	}

	inline vec3 mat2_homogeneous_rotation_mulptiplication(vec3 vector_0, float angle)
	{
		mat3 matrix = mat3_homogenous_rotation(angle);
		vec3 out_vec3;
		for (auto i = 0; i < 3; i++)
		{
			float value = 0.0f;
			for (auto j = 0; j < 3; j++)
			{
				value += matrix.elements[i][j] * vector_0.elements[j];
			}
			out_vec3.elements[i] = value;
		}
		return out_vec3;
	}

	inline vec3 mat2_homogenous_translation_multiplication(vec3 vector_0, float x, float y)
	{
		mat3 matrix = mat3_homogenous_translation(x, y);
		vec3 out_vec3;
		for (auto i = 0; i < 3; i++)
		{
			float value = 0.0f;
			for (auto j = 0; j < 3; j++)
			{
				value += matrix.elements[i][j] * vector_0.elements[j];
			}
			out_vec3.elements[i] = value;
		}
		return out_vec3;
	}

	inline vec3 mat2_homogenous_scale_multiplication(vec3 vector_0, float x, float y, float z)
	{
		mat3 matrix = mat3_homogenous_scale(x, y, z);
		vec3 out_vec3;
		for (auto i = 0; i < 3; i++)
		{
			float value = 0.0f;
			for (auto j = 0; j < 3; j++)
			{
				value += matrix.elements[i][j] * vector_0.elements[j];
			}
			out_vec3.elements[i] = value;
		}
		return out_vec3;
	}

	inline vec2 mat2_rotation_multiplication(vec2 vector_0, float angle)
	{
		mat2 out_mat2;
		vec2 out_vec2;
		out_mat2.a1 = vec2_create(dcos(angle), -dsin(angle));
		out_mat2.a2 = vec2_create(dsin(angle), dcos(angle));
		for (auto i = 0; i < 2; i++)
		{
			float value = 0.0f;
			for (auto j = 0; j < 2; j++)
			{
				value += out_mat2.elements[i][j] * vector_0.elements[j];
			}
			out_vec2.elements[i] = value;
		}
		return out_vec2;
	}

	////==============================mat3================================

	inline mat3 mat3_multiply(mat3 matrix_0, mat3 matrix_1)
	{
		mat3 out_mat3;
		for (auto i = 0; i < 3; i++)
		{
			for (auto j = 0; j < 3; j++)
			{
				float value = 0.0;
				for (auto k = 0; k < 3; k++)
				{
					value += matrix_0.elements[i][k] * matrix_1.elements[k][j];
				}
				out_mat3.elements[i][j] = value;
			}
		}
		return out_mat3;
	}
	inline mat3 mat3_scale(mat3 matrix_0, float factor)
	{
		for (auto i = 0; i < 3; i++)
		{
			for (auto j = 0; j < 3; j++)
			{
				matrix_0.elements[i][j] = matrix_0.elements[i][j] * factor;
			}
		}
		return matrix_0;
	}

	////==============================mat4================================
	inline mat4 mat4_zero()
	{
		mat4 out_mat4;
		out_mat4.a1 = vec4_copy(vec4_zero());
		out_mat4.a2 = vec4_copy(vec4_zero());
		out_mat4.a3 = vec4_copy(vec4_zero());
		out_mat4.a4 = vec4_copy(vec4_zero());
		return out_mat4;
	}
	inline mat4 mat4_transponse(mat4 matrix_0)
	{
		mat4 result;
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
			{
				result.elements[j][i] = matrix_0.elements[i][j];
			}
		}
		return result;
	}

	inline mat4 mat4_identity()
	{
		mat4 out_mat4 = mat4_zero();
		out_mat4.a1.elements[0] = 1.0f;
		out_mat4.a2.elements[1] = 1.0f;
		out_mat4.a3.elements[2] = 1.0f;
		out_mat4.a4.elements[3] = 1.0f;
		return out_mat4;
	}

	inline mat4 mat4_homogeneous_translation(float x, float y, float z)
	{
		mat4 out_mat4_h = mat4_identity();
		out_mat4_h.a1.elements[3] = x;
		out_mat4_h.a2.elements[3] = y;
		out_mat4_h.a3.elements[3] = z;
		return out_mat4_h;
	}
	inline mat4 mat4_homogeneous_rotation(const char *axis, float angle)
	{
		mat4 matrix = mat4_identity();
		if ((strcmp(axis, "x") == 0) || (strcmp(axis, "X") == 0))
		{
			matrix.a2.elements[1] = dcos(angle);
			matrix.a2.elements[2] = dsin(angle);
			matrix.a3.elements[1] = -(dsin(angle));
			matrix.a3.elements[2] = dcos(angle);
		}
		else if ((strcmp(axis, "y") == 0) || (strcmp(axis, "Y") == 0))
		{
			matrix.a1.elements[0] = dcos(angle);
			matrix.a1.elements[2] = -(dsin(angle));
			matrix.a3.elements[0] = dsin(angle);
			matrix.a3.elements[2] = dcos(angle);
		}
		else if ((strcmp(axis, "z") == 0) || (strcmp(axis, "Z") == 0))
		{
			matrix.a1.elements[0] = dcos(angle);
			matrix.a1.elements[1] = dsin(angle);
			matrix.a2.elements[0] = -(dsin(angle));
			matrix.a2.elements[1] = dcos(angle);
		}
		return matrix;
	}
	inline mat4 mat4_homogenous_scale(float x, float y, float z, float w)
	{
		mat4 out_mat4 = mat4_identity();
		out_mat4.a1.elements[0] = x;
		out_mat4.a2.elements[1] = y;
		out_mat4.a3.elements[2] = z;
		out_mat4.a4.elements[3] = w;
		return out_mat4;
	}

	inline mat4 mat4_multiply(mat4 matrix_0, mat4 matrix_1)
	{
		mat4 out_mat4;
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
			{
				float value = 0.0;
				for (auto k = 0; k < 4; k++)
				{
					value += matrix_0.elements[i][k] * matrix_1.elements[k][j];
				}
				out_mat4.elements[i][j] = value;
			}
		}
		return out_mat4;
	}

	inline mat4 operator*(mat4 matrix_0, mat4 matrix_1)
	{
		mat4 out_mat4;
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
			{
				float value = 0.0;
				for (auto k = 0; k < 4; k++)
				{
					value += matrix_0.elements[i][k] * matrix_1.elements[k][j];
				}
				out_mat4.elements[i][j] = value;
			}
		}
		return out_mat4;
	}

	inline mat4 mat4_scale(mat4 matrix_0, float factor)
	{
		for (auto i = 0; i < 4; i++)
		{
			for (auto j = 0; j < 4; j++)
			{
				matrix_0.elements[i][j] = matrix_0.elements[i][j] * factor;
			}
		}
		return matrix_0;
	}

	inline mat4 mat4_orthographic_proj(float right, float left, float top, float bottom, float nearclip, float farclip)
	{
		float t1 = (right + left) / 2;

		float t2 = (bottom + top) / 2;

		float t3 = nearclip;

		float s1 = 2.0f / (right - left);

		float s2 = 2.0f / (bottom - top);

		float s3 = 1.0f / (farclip - nearclip);

		mat4 orthographic_matrix = mat4_identity();

        orthographic_matrix = mat4_transponse(orthographic_matrix);
		
		mat4 transmatrix = mat4_homogeneous_translation(-t1, -t2, -t3);

		mat4 scalematrix = mat4_homogenous_scale(s1, s2, s3, 1.0f);

		mat4 orthographic_proj = mat4_multiply(scalematrix, transmatrix);
         
		return orthographic_matrix;
	}
	inline mat4 mat4_perspective(float fov, float aspect_ratio, float nearclip, float farclip)
	{
		/*	float fov_rad = fov * DEGRAD_MUTIPLE;*/
		float tanhalf = dtan(fov / 2.0);
		mat4 result = mat4_zero();
		result.elements[0][0] = 1.0f / (aspect_ratio * tanhalf);
		result.elements[1][1] = (1.0f / (tanhalf));
		result.elements[2][2] = (farclip) / (farclip - nearclip);
		result.elements[2][3] = -(nearclip * farclip) / (farclip - nearclip);
		result.elements[3][2] = 1.0f;
		dlm::mat4 X = dlm::mat4_homogeneous_rotation("x", 180.0f);
		result = result * X;
		// result = mat4_transponse(result);
		return result;
	}

	inline mat4 mat4_lookAt(vec3 eye, vec3 center, vec3 up)
	{
		vec3 f = vec3_nomalize(center - eye);
		vec3 r = vec3_nomalize(Cross_prod_vec3(up, f));
		vec3 u = (Cross_prod_vec3(f, r));

		mat4 Result = mat4_identity();
		Result.elements[0][0] = r.x;
		Result.elements[1][0] = r.y;
		Result.elements[2][0] = r.z;
		Result.elements[0][1] = u.x;
		Result.elements[1][1] = u.y;
		Result.elements[2][1] = u.z;
		Result.elements[0][2] = f.x;
		Result.elements[1][2] = f.y;
		Result.elements[2][2] = f.z;
		Result.elements[3][0] = -Dot_prod_vec3(r, eye);
		Result.elements[3][1] = -Dot_prod_vec3(u, eye);
		Result.elements[3][2] = -Dot_prod_vec3(f, eye);
		Result = mat4_transponse(Result);
		return Result;
	}

	inline float mat2_det(mat2 matrix)
	{
		float det = (matrix.a1.elements[0] * matrix.a2.elements[1]) - (matrix.a1.elements[1] * matrix.a2.elements[0]);
		return det;
	}

	inline float mat3_det(mat3 matrix)
	{
		mat2 first_column;
		first_column.a1.elements[0] = matrix.a2.elements[1];
		first_column.a1.elements[1] = matrix.a2.elements[2];
		first_column.a2.elements[0] = matrix.a3.elements[1];
		first_column.a2.elements[1] = matrix.a3.elements[2];
		mat2 second_column;
		second_column.a1.elements[0] = matrix.a2.elements[0];
		second_column.a1.elements[1] = matrix.a2.elements[2];
		second_column.a2.elements[0] = matrix.a3.elements[0];
		second_column.a2.elements[1] = matrix.a3.elements[2];

		mat2 third_column;
		third_column.a1.elements[0] = matrix.a2.elements[0];
		third_column.a1.elements[1] = matrix.a2.elements[1];
		third_column.a2.elements[0] = matrix.a3.elements[0];
		third_column.a2.elements[1] = matrix.a3.elements[1];
		float det = (matrix.a1.elements[0] * mat2_det(first_column)) - (matrix.a1.elements[1] * mat2_det(second_column)) + (matrix.a1.elements[2] * mat2_det(third_column));
		return det;
	}

	inline float mat4_det(mat4 matrix)
	{
		mat3 first_column;
		first_column.a1.elements[0] = matrix.a2.elements[1];
		first_column.a1.elements[1] = matrix.a2.elements[2];
		first_column.a1.elements[2] = matrix.a2.elements[3];
		first_column.a2.elements[0] = matrix.a3.elements[1];
		first_column.a2.elements[1] = matrix.a3.elements[2];
		first_column.a2.elements[2] = matrix.a3.elements[3];
		first_column.a3.elements[0] = matrix.a4.elements[1];
		first_column.a3.elements[1] = matrix.a4.elements[2];
		first_column.a3.elements[2] = matrix.a4.elements[3];
		mat3 second_column;
		second_column.a1.elements[0] = matrix.a2.elements[0];
		second_column.a1.elements[1] = matrix.a2.elements[2];
		second_column.a1.elements[2] = matrix.a2.elements[3];
		second_column.a2.elements[0] = matrix.a3.elements[0];
		second_column.a2.elements[1] = matrix.a3.elements[2];
		second_column.a2.elements[2] = matrix.a3.elements[3];
		second_column.a3.elements[0] = matrix.a4.elements[0];
		second_column.a3.elements[1] = matrix.a4.elements[2];
		second_column.a3.elements[2] = matrix.a4.elements[3];
		mat3 third_column;
		third_column.a1.elements[0] = matrix.a2.elements[0];
		third_column.a1.elements[1] = matrix.a2.elements[1];
		third_column.a1.elements[2] = matrix.a2.elements[3];
		third_column.a2.elements[0] = matrix.a3.elements[0];
		third_column.a2.elements[1] = matrix.a3.elements[1];
		third_column.a2.elements[2] = matrix.a3.elements[3];
		third_column.a3.elements[0] = matrix.a4.elements[0];
		third_column.a3.elements[1] = matrix.a4.elements[1];
		third_column.a3.elements[2] = matrix.a4.elements[3];
		mat3 fourth_column;
		fourth_column.a1.elements[0] = matrix.a2.elements[0];
		fourth_column.a1.elements[1] = matrix.a2.elements[1];
		fourth_column.a1.elements[2] = matrix.a2.elements[2];
		fourth_column.a2.elements[0] = matrix.a3.elements[0];
		fourth_column.a2.elements[1] = matrix.a3.elements[1];
		fourth_column.a2.elements[2] = matrix.a3.elements[2];
		fourth_column.a3.elements[0] = matrix.a4.elements[0];
		fourth_column.a3.elements[1] = matrix.a4.elements[1];
		fourth_column.a3.elements[2] = matrix.a4.elements[2];
		float det = (matrix.a1.elements[0] * mat3_det(first_column)) - (matrix.a1.elements[1] * mat3_det(second_column)) + (matrix.a1.elements[2] * mat3_det(third_column)) -(matrix.a1.elements[3] * mat3_det(fourth_column));
		return det;
	}

    inline vec4 mat4_vec4_mul(mat4 matrix, vec4 vector){
		vec4 result;
		for(auto i=0; i < 4; i++){
			float value = 0;
			for(auto j=0; j < 4; j++){
				value += (matrix.elements[i][j] * vector.elements[j]);
			}
			result.elements[i]= value;
		}
		return result;
	}

	inline mat4 mat4_inverse(mat4 matrix)
	{
		float det = mat4_det(matrix);
		mat3 M11;
		M11.a1.elements[0] = matrix.a2.elements[1];
		M11.a1.elements[1] = matrix.a2.elements[2];
		M11.a1.elements[2] = matrix.a2.elements[3];
		M11.a2.elements[0] = matrix.a3.elements[1];
		M11.a2.elements[1] = matrix.a3.elements[2];
		M11.a2.elements[2] = matrix.a3.elements[3];
		M11.a3.elements[0] = matrix.a4.elements[1];
		M11.a3.elements[1] = matrix.a4.elements[2];
		M11.a3.elements[2] = matrix.a4.elements[3];
		mat3 M12;
		M12.a1.elements[0] = matrix.a2.elements[0];
		M12.a1.elements[1] = matrix.a2.elements[2];
		M12.a1.elements[2] = matrix.a2.elements[3];
		M12.a2.elements[0] = matrix.a3.elements[0];
		M12.a2.elements[1] = matrix.a3.elements[2];
		M12.a2.elements[2] = matrix.a3.elements[3];
		M12.a3.elements[0] = matrix.a4.elements[0];
		M12.a3.elements[1] = matrix.a4.elements[2];
		M12.a3.elements[2] = matrix.a4.elements[3];
		mat3 M13;
		M13.a1.elements[0] = matrix.a2.elements[0];
		M13.a1.elements[1] = matrix.a2.elements[1];
		M13.a1.elements[2] = matrix.a2.elements[3];
		M13.a2.elements[0] = matrix.a3.elements[0];
		M13.a2.elements[1] = matrix.a3.elements[1];
		M13.a2.elements[2] = matrix.a3.elements[3];
		M13.a3.elements[0] = matrix.a4.elements[0];
		M13.a3.elements[1] = matrix.a4.elements[1];
		M13.a3.elements[2] = matrix.a4.elements[3];
		mat3 M14;
		M14.a1.elements[0] = matrix.a2.elements[0];
		M14.a1.elements[1] = matrix.a2.elements[1];
		M14.a1.elements[2] = matrix.a2.elements[2];
		M14.a2.elements[0] = matrix.a3.elements[0];
		M14.a2.elements[1] = matrix.a3.elements[1];
		M14.a2.elements[2] = matrix.a3.elements[2];
		M14.a3.elements[0] = matrix.a4.elements[0];
		M14.a3.elements[1] = matrix.a4.elements[1];
		M14.a3.elements[2] = matrix.a4.elements[2];
		mat3 M21;
		M21.a1.elements[0] = matrix.a1.elements[1];
		M21.a1.elements[1] = matrix.a1.elements[2];
		M21.a1.elements[2] = matrix.a1.elements[3];
		M21.a2.elements[0] = matrix.a3.elements[1];
		M21.a2.elements[1] = matrix.a3.elements[2];
		M21.a2.elements[2] = matrix.a3.elements[3];
		M21.a3.elements[0] = matrix.a4.elements[1];
		M21.a3.elements[1] = matrix.a4.elements[2];
		M21.a3.elements[2] = matrix.a4.elements[3];
		mat3 M22;
		M22.a1.elements[0] = matrix.a1.elements[0];
		M22.a1.elements[1] = matrix.a1.elements[2];
		M22.a1.elements[2] = matrix.a1.elements[3];
		M22.a2.elements[0] = matrix.a3.elements[0];
		M22.a2.elements[1] = matrix.a3.elements[2];
		M22.a2.elements[2] = matrix.a3.elements[3];
		M22.a3.elements[0] = matrix.a4.elements[0];
		M22.a3.elements[1] = matrix.a4.elements[2];
		M22.a3.elements[2] = matrix.a4.elements[3];
		mat3 M23;
		M23.a1.elements[0] = matrix.a1.elements[0];
		M23.a1.elements[1] = matrix.a1.elements[1];
		M23.a1.elements[2] = matrix.a1.elements[3];
		M23.a2.elements[0] = matrix.a3.elements[0];
		M23.a2.elements[1] = matrix.a3.elements[1];
		M23.a2.elements[2] = matrix.a3.elements[3];
		M23.a3.elements[0] = matrix.a4.elements[0];
		M23.a3.elements[1] = matrix.a4.elements[1];
		M23.a3.elements[2] = matrix.a4.elements[3];
		mat3 M24;
		M24.a1.elements[0] = matrix.a1.elements[0];
		M24.a1.elements[1] = matrix.a1.elements[1];
		M24.a1.elements[2] = matrix.a1.elements[2];
		M24.a2.elements[0] = matrix.a3.elements[0];
		M24.a2.elements[1] = matrix.a3.elements[1];
		M24.a2.elements[2] = matrix.a3.elements[2];
		M24.a3.elements[0] = matrix.a4.elements[0];
		M24.a3.elements[1] = matrix.a4.elements[1];
		M24.a3.elements[2] = matrix.a4.elements[2];
		mat3 M31;
		M31.a1.elements[0] = matrix.a1.elements[1];
		M31.a1.elements[1] = matrix.a1.elements[2];
		M31.a1.elements[2] = matrix.a1.elements[3];
		M31.a2.elements[0] = matrix.a2.elements[1];
		M31.a2.elements[1] = matrix.a2.elements[2];
		M31.a2.elements[2] = matrix.a2.elements[3];
		M31.a3.elements[0] = matrix.a4.elements[1];
		M31.a3.elements[1] = matrix.a4.elements[2];
		M31.a3.elements[2] = matrix.a4.elements[3];
		mat3 M32;
		M32.a1.elements[0] = matrix.a1.elements[0];
		M32.a1.elements[1] = matrix.a1.elements[2];
		M32.a1.elements[2] = matrix.a1.elements[3];
		M32.a2.elements[0] = matrix.a2.elements[0];
		M32.a2.elements[1] = matrix.a2.elements[2];
		M32.a2.elements[2] = matrix.a2.elements[3];
		M32.a3.elements[0] = matrix.a4.elements[0];
		M32.a3.elements[1] = matrix.a4.elements[2];
		M32.a3.elements[2] = matrix.a4.elements[3];
		mat3 M33;
		M33.a1.elements[0] = matrix.a1.elements[0];
		M33.a1.elements[1] = matrix.a1.elements[1];
		M33.a1.elements[2] = matrix.a1.elements[3];
		M33.a2.elements[0] = matrix.a2.elements[0];
		M33.a2.elements[1] = matrix.a2.elements[1];
		M33.a2.elements[2] = matrix.a2.elements[3];
		M33.a3.elements[0] = matrix.a4.elements[0];
		M33.a3.elements[1] = matrix.a4.elements[1];
		M33.a3.elements[2] = matrix.a4.elements[3];
		mat3 M34;
		M34.a1.elements[0] = matrix.a1.elements[0];
		M34.a1.elements[1] = matrix.a1.elements[1];
		M34.a1.elements[2] = matrix.a1.elements[2];
		M34.a2.elements[0] = matrix.a2.elements[0];
		M34.a2.elements[1] = matrix.a2.elements[1];
		M34.a2.elements[2] = matrix.a2.elements[2];
		M34.a3.elements[0] = matrix.a4.elements[0];
		M34.a3.elements[1] = matrix.a4.elements[1];
		M34.a3.elements[2] = matrix.a4.elements[2];
		mat3 M41;
		M41.a1.elements[0] = matrix.a1.elements[1];
		M41.a1.elements[1] = matrix.a1.elements[2];
		M41.a1.elements[2] = matrix.a1.elements[3];
		M41.a2.elements[0] = matrix.a2.elements[1];
		M41.a2.elements[1] = matrix.a2.elements[2];
		M41.a2.elements[2] = matrix.a2.elements[3];
		M41.a3.elements[0] = matrix.a3.elements[1];
		M41.a3.elements[1] = matrix.a3.elements[2];
		M41.a3.elements[2] = matrix.a3.elements[3];
		mat3 M42;
		M42.a1.elements[0] = matrix.a1.elements[0];
		M42.a1.elements[1] = matrix.a1.elements[2];
		M42.a1.elements[2] = matrix.a1.elements[3];
		M42.a2.elements[0] = matrix.a2.elements[0];
		M42.a2.elements[1] = matrix.a2.elements[2];
		M42.a2.elements[2] = matrix.a2.elements[3];
		M42.a3.elements[0] = matrix.a3.elements[0];
		M42.a3.elements[1] = matrix.a3.elements[2];
		M42.a3.elements[2] = matrix.a3.elements[3];
		mat3 M43;
		M43.a1.elements[0] = matrix.a1.elements[0];
		M43.a1.elements[1] = matrix.a1.elements[1];
		M43.a1.elements[2] = matrix.a1.elements[3];
		M43.a2.elements[0] = matrix.a2.elements[0];
		M43.a2.elements[1] = matrix.a2.elements[1];
		M43.a2.elements[2] = matrix.a2.elements[3];
		M43.a3.elements[0] = matrix.a3.elements[0];
		M43.a3.elements[1] = matrix.a3.elements[1];
		M43.a3.elements[2] = matrix.a3.elements[3];
		mat3 M44;
		M44.a1.elements[0] = matrix.a1.elements[0];
		M44.a1.elements[1] = matrix.a1.elements[1];
		M44.a1.elements[2] = matrix.a1.elements[2];
		M44.a2.elements[0] = matrix.a2.elements[0];
		M44.a2.elements[1] = matrix.a2.elements[1];
		M44.a2.elements[2] = matrix.a2.elements[2];
		M44.a3.elements[0] = matrix.a3.elements[0];
		M44.a3.elements[1] = matrix.a3.elements[1];
		M44.a3.elements[2] = matrix.a3.elements[2];

		mat4 Adjugate ;
		Adjugate.a1.elements[0]= +(mat3_det(M11));
		Adjugate.a1.elements[1]= -(mat3_det(M12));
		Adjugate.a1.elements[2]= +(mat3_det(M13));
		Adjugate.a1.elements[3]= -(mat3_det(M14));
		Adjugate.a2.elements[0]= -(mat3_det(M21));
		Adjugate.a2.elements[1]= +(mat3_det(M22));
		Adjugate.a2.elements[2]= -(mat3_det(M23));
		Adjugate.a2.elements[3]= +(mat3_det(M24));
		Adjugate.a3.elements[0]= +(mat3_det(M31));
		Adjugate.a3.elements[1]= -(mat3_det(M32));
		Adjugate.a3.elements[2]= +(mat3_det(M33));
		Adjugate.a3.elements[3]= -(mat3_det(M34));
		Adjugate.a4.elements[0]= -(mat3_det(M41));
		Adjugate.a4.elements[1]= +(mat3_det(M42));
		Adjugate.a4.elements[2]= -(mat3_det(M43));
		Adjugate.a4.elements[3]= +(mat3_det(M44));
		Adjugate = mat4_transponse(Adjugate);
		float factor = 1.0f/det;
		mat4 inverse_matrix = mat4_scale(Adjugate,factor);
		return inverse_matrix;
	}


	inline mat4 mat4_euler_xyz(float x, float y, float z)
	{
		//vec4 camera_position
		// we need to get the rotational mat4 matrix for the x y and z
	 mat4 R = mat4_homogeneous_rotation("x", x) * mat4_homogeneous_rotation("y", y) * mat4_homogeneous_rotation("z", z);
	//  vec4 forward_vec = mat4_vec4_mul(R,vec4_create(0.0f,0.0f,1.0f, 0.0f));
	//  vec4 right_vec = mat4_vec4_mul(R,vec4_create(1.0f,0.0f,0.0f,0.0f));
	//  vec4 up_vec = mat4_vec4_mul(R,vec4_create(0.0f,1.0f,0.0f,0.0f));
	//  mat4 view;
	//  view = mat4_identity();
	//  view.elements[0][0]= right_vec.elements[0];
	//  view.elements[1][0]= right_vec.elements[1];
	//  view.elements[2][0]= right_vec.elements[2];
	//  view.elements[3][0]= -(Dot_prod_vec4(right_vec, camera_position));
	//  view.elements[0][1]=  up_vec.elements[0];
	//  view.elements[1][1]=  up_vec.elements[1];
	//  view.elements[2][1]=  up_vec.elements[2];
	//  view.elements[3][1]= -(Dot_prod_vec4(up_vec, camera_position));
	//  view.elements[0][2]= forward_vec.elements[0];
	//  view.elements[1][2]= forward_vec.elements[1];
	//  view.elements[2][2]= forward_vec.elements[2];
	//  view.elements[3][2]= -(Dot_prod_vec4(forward_vec, camera_position));

    //  view= mat4_transponse(view);
		
	 return R;	
	}
	inline vec3 mat_4_forward_vec(mat4 view){
		vec3 forward_vector = vec3_zero();
		forward_vector.elements[0]= -(view.a1.elements[2]);
		forward_vector.elements[1]= -(view.a2.elements[2]);
		forward_vector.elements[2]= -(view.a3.elements[2]);
		return forward_vector;

	}
   inline vec3 mat_4_backward_vec(mat4 view){
		vec3 backward_vector = vec3_zero();
		backward_vector.elements[0]=view.a1.elements[2];
		backward_vector.elements[1]=view.a2.elements[2];
		backward_vector.elements[2]=view.a3.elements[2];
		return backward_vector;

	}
    inline vec3 mat_4_left_vec(mat4 view){
		vec3 left_vector = vec3_zero();
		left_vector.elements[0]=-(view.a1.elements[0]);
		left_vector.elements[1]=-(view.a2.elements[0]);
		left_vector.elements[2]=-(view.a3.elements[0]);
		return left_vector;

	}
	 inline vec3 mat_4_right_vec(mat4 view){
		vec3 right_vector = vec3_zero();
		right_vector.elements[0]=view.a1.elements[0];
		right_vector.elements[1]=view.a2.elements[0];
		right_vector.elements[2]=view.a3.elements[0];
		return right_vector;

	}
	 inline vec3 mat_4_down_vec(mat4 view){
		vec3 down_vector = vec3_zero();
		down_vector.elements[0]=-(view.a1.elements[1]);
		down_vector.elements[1]=-(view.a2.elements[1]);
		down_vector.elements[2]=-(view.a3.elements[1]);
		return down_vector;

	}
	 inline vec3 mat_up_vec(mat4 view){
		vec3 up_vector = vec3_zero();
		up_vector.elements[0]=view.a1.elements[1];
		up_vector.elements[1]=view.a2.elements[1];
		up_vector.elements[2]=view.a3.elements[1];
		return up_vector;

	}
	

};