/* 
 * Copyright (c) 2008 MUSIC TECHNOLOGY GROUP (MTG)
 *                         UNIVERSITAT POMPEU FABRA 
 * 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * 
 */
#include "sms.h"

#define SIN_TABLE_SIZE 4096//was 2046

static double fSineScale;
float *sms_tab_sine;

/* clear sine table */
void ClearSine()
{
  if(sms_tab_sine)
    free(sms_tab_sine);
  sms_tab_sine = 0;
}

/* prepares the sine table, returns 1 if allocations made, 0 on failure
 * int nTableSize;    size of table
 */
int PrepSine (int nTableSize)
{
  register int i;
  double fTheta;
  
  if((sms_tab_sine = (float *)malloc(SIN_TABLE_SIZE*sizeof(float))) == 0)
    return (0);
  //nSineTabSize = SIN_TABLE_SIZE;
  fSineScale =  (double)(TWO_PI) / (double)(SIN_TABLE_SIZE - 1);
  fTheta = 0.0;
  for(i = 0; i < SIN_TABLE_SIZE; i++) 
  {
    fTheta = fSineScale * (double)i;
    sms_tab_sine[i] = sin(fTheta);
  }
  return (1);
}

/* function that returns approximately sin(fTheta)
 * double fTheta;    angle in radians
 */
double SinTab (double fTheta)
{
  double fSign = 1.0, fT;
  int i;
  
  fTheta = fTheta - floor(fTheta / TWO_PI) * TWO_PI;
  
  if(fTheta < 0)
  {
    fSign = -1;
    fTheta = -fTheta;
  }
  
  i = fTheta / fSineScale + .5;
  fT = sms_tab_sine[i];
  
  if (fSign == 1)
    return(fT);
  else
    return(-fT);
}
