/*
Copyright (C) 1996-2001 Id Software, Inc.
Copyright (C) 2002-2009 John Fitzgibbons and others
Copyright (C) 2007-2008 Kristian Duske
Copyright (C) 2010-2014 QuakeSpasm developers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// mathlib.c -- math primitives

#include "quakedef.h"

vec3_t vec3_origin = {0,0,0};

/*-----------------------------------------------------------------*/


//#define DEG2RAD( a ) ( a * M_PI ) / 180.0F
#define DEG2RAD( a ) ( (a) * M_PI_DIV_180 ) //johnfitz

void ProjectPointOnPlane( vec3_t dst, const vec3_t p, const vec3_t normal )
{
	float d;
	vec3_t n;
	float inv_denom;

	inv_denom = 1.0F / DotProduct( normal, normal );

	d = DotProduct( normal, p ) * inv_denom;

	n[0] = normal[0] * inv_denom;
	n[1] = normal[1] * inv_denom;
	n[2] = normal[2] * inv_denom;

	dst[0] = p[0] - d * n[0];
	dst[1] = p[1] - d * n[1];
	dst[2] = p[2] - d * n[2];
}

/*
** assumes "src" is normalized
*/
void PerpendicularVector( vec3_t dst, const vec3_t src )
{
	int	pos;
	int i;
	float minelem = 1.0F;
	vec3_t tempvec;

	/*
	** find the smallest magnitude axially aligned vector
	*/
	for ( pos = 0, i = 0; i < 3; i++ )
	{
		if ( fabs( src[i] ) < minelem )
		{
			pos = i;
			minelem = fabs( src[i] );
		}
	}
	tempvec[0] = tempvec[1] = tempvec[2] = 0.0F;
	tempvec[pos] = 1.0F;

	/*
	** project the point onto the plane defined by src
	*/
	ProjectPointOnPlane( dst, tempvec, src );

	/*
	** normalize the result
	*/
	VectorNormalize( dst );
}

//johnfitz -- removed RotatePointAroundVector() becuase it's no longer used and my compiler fucked it up anyway

//spike -- readded, because it is useful, and my version of gcc has never had a problem with it.
void RotatePointAroundVector( vec3_t dst, const vec3_t dir, const vec3_t point, float degrees )
{
	float	m[3][3];
	float	im[3][3];
	float	zrot[3][3];
	float	tmpmat[3][3];
	float	rot[3][3];
	int	i;
	vec3_t vr, vu, vf;

	vf[0] = dir[0];
	vf[1] = dir[1];
	vf[2] = dir[2];

	PerpendicularVector( vr, dir );
	CrossProduct( vr, vf, vu );

	m[0][0] = vr[0];
	m[1][0] = vr[1];
	m[2][0] = vr[2];

	m[0][1] = vu[0];
	m[1][1] = vu[1];
	m[2][1] = vu[2];

	m[0][2] = vf[0];
	m[1][2] = vf[1];
	m[2][2] = vf[2];

	memcpy( im, m, sizeof( im ) );

	im[0][1] = m[1][0];
	im[0][2] = m[2][0];
	im[1][0] = m[0][1];
	im[1][2] = m[2][1];
	im[2][0] = m[0][2];
	im[2][1] = m[1][2];

	memset( zrot, 0, sizeof( zrot ) );
	zrot[0][0] = zrot[1][1] = zrot[2][2] = 1.0F;

	zrot[0][0] = cos( DEG2RAD( degrees ) );
	zrot[0][1] = sin( DEG2RAD( degrees ) );
	zrot[1][0] = -sin( DEG2RAD( degrees ) );
	zrot[1][1] = cos( DEG2RAD( degrees ) );

	R_ConcatRotations( m, zrot, tmpmat );
	R_ConcatRotations( tmpmat, im, rot );

	for ( i = 0; i < 3; i++ )
	{
		dst[i] = rot[i][0] * point[0] + rot[i][1] * point[1] + rot[i][2] * point[2];
	}
}
/*-----------------------------------------------------------------*/


float	anglemod(float a)
{
#if 0
	if (a >= 0)
		a -= 360*(int)(a/360);
	else
		a += 360*( 1 + (int)(-a/360) );
#endif
	a = (360.0/65536) * ((int)(a*(65536/360.0)) & 65535);
	return a;
}


/*
==================
BoxOnPlaneSide

Returns 1, 2, or 1 + 2
==================
*/
int BoxOnPlaneSide (vec3_t emins, vec3_t emaxs, mplane_t *p)
{
	float	dist1, dist2;
	int		sides;

#if 0	// this is done by the BOX_ON_PLANE_SIDE macro before calling this
		// function
// fast axial cases
	if (p->type < 3)
	{
		if (p->dist <= emins[p->type])
			return 1;
		if (p->dist >= emaxs[p->type])
			return 2;
		return 3;
	}
#endif

// general case
	switch (p->signbits)
	{
	case 0:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 1:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		break;
	case 2:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 3:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		break;
	case 4:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 5:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emaxs[2];
		break;
	case 6:
		dist1 = p->normal[0]*emaxs[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emins[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	case 7:
		dist1 = p->normal[0]*emins[0] + p->normal[1]*emins[1] + p->normal[2]*emins[2];
		dist2 = p->normal[0]*emaxs[0] + p->normal[1]*emaxs[1] + p->normal[2]*emaxs[2];
		break;
	default:
		dist1 = dist2 = 0;		// shut up compiler
		Sys_Error ("BoxOnPlaneSide:  Bad signbits");
		break;
	}

#if 0
	int		i;
	vec3_t	corners[2];

	for (i=0 ; i<3 ; i++)
	{
		if (plane->normal[i] < 0)
		{
			corners[0][i] = emins[i];
			corners[1][i] = emaxs[i];
		}
		else
		{
			corners[1][i] = emins[i];
			corners[0][i] = emaxs[i];
		}
	}
	dist = DotProduct (plane->normal, corners[0]) - plane->dist;
	dist2 = DotProduct (plane->normal, corners[1]) - plane->dist;
	sides = 0;
	if (dist1 >= 0)
		sides = 1;
	if (dist2 < 0)
		sides |= 2;
#endif

	sides = 0;
	if (dist1 >= p->dist)
		sides = 1;
	if (dist2 < p->dist)
		sides |= 2;

#ifdef PARANOID
	if (sides == 0)
		Sys_Error ("BoxOnPlaneSide: sides==0");
#endif

	return sides;
}

//johnfitz -- the opposite of AngleVectors.  this takes forward and generates pitch yaw roll
//Spike: take right and up vectors to properly set yaw and roll
void VectorAngles (const vec3_t forward, float *up, vec3_t angles)
{
	if (forward[0] == 0 && forward[1] == 0)
	{	//either vertically up or down
		if (forward[2] > 0)
		{
			angles[PITCH] = -90;
			angles[YAW] = up ? atan2(-up[1], -up[0]) / M_PI_DIV_180: 0;
		}
		else
		{
			angles[PITCH] = 90;
			angles[YAW] = up ? atan2(up[1], up[0]) / M_PI_DIV_180: 0;
		}
		angles[ROLL] = 0;
	}
	else
	{
		angles[PITCH] = -atan2(forward[2], sqrt(DotProduct2(forward,forward)));
		angles[YAW] = atan2(forward[1], forward[0]);

		if (up)
		{
			vec_t cp = cos(angles[PITCH]), sp = sin(angles[PITCH]);
			vec_t cy = cos(angles[YAW]), sy = sin(angles[YAW]);
			vec3_t tleft, tup;
			tleft[0] = -sy;
			tleft[1] = cy;
			tleft[2] = 0;
			tup[0] = sp*cy;
			tup[1] = sp*sy;
			tup[2] = cp;
			angles[ROLL] = -atan2(DotProduct(up, tleft), DotProduct(up, tup)) / M_PI_DIV_180;
		}
		else angles[ROLL] = 0;

		angles[PITCH] /= M_PI_DIV_180;
		angles[YAW] /= M_PI_DIV_180;
	}
}

void AngleVectors (vec3_t angles, vec3_t forward, vec3_t right, vec3_t up)
{
	float		angle;
	float		sr, sp, sy, cr, cp, cy;

	angle = angles[YAW] * (M_PI*2 / 360);
	sy = sin(angle);
	cy = cos(angle);
	angle = angles[PITCH] * (M_PI*2 / 360);
	sp = sin(angle);
	cp = cos(angle);
	angle = angles[ROLL] * (M_PI*2 / 360);
	sr = sin(angle);
	cr = cos(angle);

	forward[0] = cp*cy;
	forward[1] = cp*sy;
	forward[2] = -sp;
	right[0] = (-1*sr*sp*cy+-1*cr*-sy);
	right[1] = (-1*sr*sp*sy+-1*cr*cy);
	right[2] = -1*sr*cp;
	up[0] = (cr*sp*cy+-sr*-sy);
	up[1] = (cr*sp*sy+-sr*cy);
	up[2] = cr*cp;
}

int VectorCompare (vec3_t v1, vec3_t v2)
{
	int		i;

	for (i=0 ; i<3 ; i++)
		if (v1[i] != v2[i])
			return 0;

	return 1;
}

void VectorMA (vec3_t veca, float scale, vec3_t vecb, vec3_t vecc)
{
	vecc[0] = veca[0] + scale*vecb[0];
	vecc[1] = veca[1] + scale*vecb[1];
	vecc[2] = veca[2] + scale*vecb[2];
}


vec_t _DotProduct (vec3_t v1, vec3_t v2)
{
	return v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
}

void _VectorSubtract (vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]-vecb[0];
	out[1] = veca[1]-vecb[1];
	out[2] = veca[2]-vecb[2];
}

void _VectorAdd (vec3_t veca, vec3_t vecb, vec3_t out)
{
	out[0] = veca[0]+vecb[0];
	out[1] = veca[1]+vecb[1];
	out[2] = veca[2]+vecb[2];
}

void _VectorCopy (vec3_t in, vec3_t out)
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];
}

void CrossProduct (const vec3_t v1, const vec3_t v2, vec3_t cross)
{
	cross[0] = v1[1]*v2[2] - v1[2]*v2[1];
	cross[1] = v1[2]*v2[0] - v1[0]*v2[2];
	cross[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

vec_t VectorLength(vec3_t v)
{
	return sqrt(DotProduct(v,v));
}

float VectorNormalize (vec3_t v)
{
	float	length, ilength;

	length = sqrt(DotProduct(v,v));

	if (length)
	{
		ilength = 1/length;
		v[0] *= ilength;
		v[1] *= ilength;
		v[2] *= ilength;
	}

	return length;

}

void VectorInverse (vec3_t v)
{
	v[0] = -v[0];
	v[1] = -v[1];
	v[2] = -v[2];
}

void VectorScale (vec3_t in, vec_t scale, vec3_t out)
{
	out[0] = in[0]*scale;
	out[1] = in[1]*scale;
	out[2] = in[2]*scale;
}


int Q_log2(int val)
{
	int answer=0;
	while (val>>=1)
		answer++;
	return answer;
}

int Q_nextPow2(int val)
{
	val--;
	val |= val >> 1;
	val |= val >> 2;
	val |= val >> 4;
	val |= val >> 8;
	val |= val >> 16;
	val++;
	return val;
}

/*
================
R_ConcatRotations
================
*/
void R_ConcatRotations (float in1[3][3], float in2[3][3], float out[3][3])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
}


/*
================
R_ConcatTransforms
================
*/
void R_ConcatTransforms (float in1[3][4], float in2[3][4], float out[3][4])
{
	out[0][0] = in1[0][0] * in2[0][0] + in1[0][1] * in2[1][0] +
				in1[0][2] * in2[2][0];
	out[0][1] = in1[0][0] * in2[0][1] + in1[0][1] * in2[1][1] +
				in1[0][2] * in2[2][1];
	out[0][2] = in1[0][0] * in2[0][2] + in1[0][1] * in2[1][2] +
				in1[0][2] * in2[2][2];
	out[0][3] = in1[0][0] * in2[0][3] + in1[0][1] * in2[1][3] +
				in1[0][2] * in2[2][3] + in1[0][3];
	out[1][0] = in1[1][0] * in2[0][0] + in1[1][1] * in2[1][0] +
				in1[1][2] * in2[2][0];
	out[1][1] = in1[1][0] * in2[0][1] + in1[1][1] * in2[1][1] +
				in1[1][2] * in2[2][1];
	out[1][2] = in1[1][0] * in2[0][2] + in1[1][1] * in2[1][2] +
				in1[1][2] * in2[2][2];
	out[1][3] = in1[1][0] * in2[0][3] + in1[1][1] * in2[1][3] +
				in1[1][2] * in2[2][3] + in1[1][3];
	out[2][0] = in1[2][0] * in2[0][0] + in1[2][1] * in2[1][0] +
				in1[2][2] * in2[2][0];
	out[2][1] = in1[2][0] * in2[0][1] + in1[2][1] * in2[1][1] +
				in1[2][2] * in2[2][1];
	out[2][2] = in1[2][0] * in2[0][2] + in1[2][1] * in2[1][2] +
				in1[2][2] * in2[2][2];
	out[2][3] = in1[2][0] * in2[0][3] + in1[2][1] * in2[1][3] +
				in1[2][2] * in2[2][3] + in1[2][3];
}


/*
===================
FloorDivMod

Returns mathematically correct (floor-based) quotient and remainder for
numer and denom, both of which should contain no fractional part. The
quotient must fit in 32 bits.
====================
*/

void FloorDivMod (double numer, double denom, int *quotient,
		int *rem)
{
	int		q, r;
	double	x;

#ifndef PARANOID
	if (denom <= 0.0)
		Sys_Error ("FloorDivMod: bad denominator %f\n", denom);

//	if ((floor(numer) != numer) || (floor(denom) != denom))
//		Sys_Error ("FloorDivMod: non-integer numer or denom %f %f\n",
//				numer, denom);
#endif

	if (numer >= 0.0)
	{

		x = floor(numer / denom);
		q = (int)x;
		r = (int)floor(numer - (x * denom));
	}
	else
	{
	//
	// perform operations with positive values, and fix mod to make floor-based
	//
		x = floor(-numer / denom);
		q = -(int)x;
		r = (int)floor(-numer - (x * denom));
		if (r != 0)
		{
			q--;
			r = (int)denom - r;
		}
	}

	*quotient = q;
	*rem = r;
}


/*
===================
GreatestCommonDivisor
====================
*/
int GreatestCommonDivisor (int i1, int i2)
{
	if (i1 > i2)
	{
		if (i2 == 0)
			return (i1);
		return GreatestCommonDivisor (i2, i1 % i2);
	}
	else
	{
		if (i1 == 0)
			return (i2);
		return GreatestCommonDivisor (i1, i2 % i1);
	}
}


/*
===================
Invert24To16

Inverts an 8.24 value to a 16.16 value
====================
*/

fixed16_t Invert24To16(fixed16_t val)
{
	if (val < 256)
		return (0xFFFFFFFF);

	return (fixed16_t)
			(((double)0x10000 * (double)0x1000000 / (double)val) + 0.5);
}

/*
===================
MatrixMultiply
====================
*/
void MatrixMultiply(float left[16], float right[16])
{
	float temp[16];
	int column, row, i;

	memcpy(temp, left, 16 * sizeof(float));
	for(row = 0; row < 4; ++row)
	{
		for(column = 0; column < 4; ++column)
		{
			float value = 0.0f;
			for (i = 0; i < 4; ++i)
				value += temp[i*4 + row] * right[column*4 + i];

			left[column * 4 + row] = value;
		}
	}
}

/*
=============
RotationMatrix
=============
*/
void RotationMatrix(float matrix[16], float angle, float x, float y, float z)
{
	const float c = cosf(angle);
	const float s = sinf(angle);

	// First column
	matrix[0*4 + 0] = x * x * (1.0f - c) + c;
	matrix[0*4 + 1] = y * x * (1.0f - c) + z * s;
	matrix[0*4 + 2] = x * z * (1.0f - c) - y * s;
	matrix[0*4 + 3] = 0.0f;

	// Second column
	matrix[1*4 + 0] = x * y * (1.0f - c) - z * s;
	matrix[1*4 + 1] = y * y * (1.0f - c) + c;
	matrix[1*4 + 2] = y * z * (1.0f - c) + x * s;
	matrix[1*4 + 3] = 0.0f;
	
	// Third column
	matrix[2*4 + 0] = x * z * (1.0f - c) + y * s;
	matrix[2*4 + 1] = y * z * (1.0f - c) - x * s;
	matrix[2*4 + 2] = z * z * (1.0f - c) + c;
	matrix[2*4 + 3] = 0.0f;

	// Fourth column
	matrix[3*4 + 0] = 0.0f;
	matrix[3*4 + 1] = 0.0f;
	matrix[3*4 + 2] = 0.0f;
	matrix[3*4 + 3] = 1.0f;
}

/*
=============
TranslationMatrix
=============
*/
void TranslationMatrix(float matrix[16], float x, float y, float z)
{
	memset(matrix, 0, 16 * sizeof(float));

	// First column
	matrix[0*4 + 0] = 1.0f;

	// Second column
	matrix[1*4 + 1] = 1.0f;
	
	// Third column
	matrix[2*4 + 2] = 1.0f;

	// Fourth column
	matrix[3*4 + 0] = x;
	matrix[3*4 + 1] = y;
	matrix[3*4 + 2] = z;
	matrix[3*4 + 3] = 1.0f;
}

/*
=============
ScaleMatrix
=============
*/
void ScaleMatrix(float matrix[16], float x, float y, float z)
{
	memset(matrix, 0, 16 * sizeof(float));

	// First column
	matrix[0*4 + 0] = x;

	// Second column
	matrix[1*4 + 1] = y;
	
	// Third column
	matrix[2*4 + 2] = z;

	// Fourth column
	matrix[3*4 + 3] = 1.0f;
}

/*
=============
IdentityMatrix
=============
*/
void IdentityMatrix(float matrix[16])
{
	memset(matrix, 0, 16 * sizeof(float));

	// First column
	matrix[0*4 + 0] = 1.0f;

	// Second column
	matrix[1*4 + 1] = 1.0f;
	
	// Third column
	matrix[2*4 + 2] = 1.0f;

	// Fourth column
	matrix[3*4 + 3] = 1.0f;
}

// Finessed from rodolphe-vaillant.fr for 4x4 matrix inversion
/*
=============
MinorMatrix
=============
*/
float MinorMatrix(float m[16], int r0, int r1, int r2, int c0, int c1, int c2)
{
	return m[4 * r0 + c0] * (m[4 * r1 + c1] * m[4 * r2 + c2] - m[4 * r2 + c1] * m[4 * r1 + c2]) -
		m[4 * r0 + c1] * (m[4 * r1 + c0] * m[4 * r2 + c2] - m[4 * r2 + c0] * m[4 * r1 + c2]) +
		m[4 * r0 + c2] * (m[4 * r1 + c0] * m[4 * r2 + c1] - m[4 * r2 + c0] * m[4 * r1 + c1]);
}

/*
=============
AdjointMatrix
=============
*/
void AdjointMatrix(float m[16], float adjOut[16])
{
	adjOut[0] = MinorMatrix(m, 1, 2, 3, 1, 2, 3); adjOut[1] = -MinorMatrix(m, 0, 2, 3, 1, 2, 3); adjOut[2] = MinorMatrix(m, 0, 1, 3, 1, 2, 3); adjOut[3] = -MinorMatrix(m, 0, 1, 2, 1, 2, 3);
	adjOut[4] = -MinorMatrix(m, 1, 2, 3, 0, 2, 3); adjOut[5] = MinorMatrix(m, 0, 2, 3, 0, 2, 3); adjOut[6] = -MinorMatrix(m, 0, 1, 3, 0, 2, 3); adjOut[7] = MinorMatrix(m, 0, 1, 2, 0, 2, 3);
	adjOut[8] = MinorMatrix(m, 1, 2, 3, 0, 1, 3); adjOut[9] = -MinorMatrix(m, 0, 2, 3, 0, 1, 3); adjOut[10] = MinorMatrix(m, 0, 1, 3, 0, 1, 3); adjOut[11] = -MinorMatrix(m, 0, 1, 2, 0, 1, 3);
	adjOut[12] = -MinorMatrix(m, 1, 2, 3, 0, 1, 2); adjOut[13] = MinorMatrix(m, 0, 2, 3, 0, 1, 2); adjOut[14] = -MinorMatrix(m, 0, 1, 3, 0, 1, 2); adjOut[15] = MinorMatrix(m, 0, 1, 2, 0, 1, 2);
}

/*
=============
DetMatrix
=============
*/
float DetMatrix(float m[16])
{
	return m[0] * MinorMatrix(m, 1, 2, 3, 1, 2, 3) -
		m[1] * MinorMatrix(m, 1, 2, 3, 0, 2, 3) +
		m[2] * MinorMatrix(m, 1, 2, 3, 0, 1, 3) -
		m[3] * MinorMatrix(m, 1, 2, 3, 0, 1, 2);
}


/*
=============
InverseMatrix
=============
*/
void InverseMatrix(float m[16], float invOut[16])
{
	AdjointMatrix(m, invOut);

	float inv_det = 1.0f / DetMatrix(m);
	for (int i = 0; i < 16; ++i)
		invOut[i] = invOut[i] * inv_det;
}

/*
=============
TransposeMatrix
=============
*/
void TransposeMatrix(float m[16], float invOut[16])
{
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			invOut[j * 4 + i] = m[i * 4 + j];
		}
	}
}
