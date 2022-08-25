/*
    tools.cpp
    Copyright (C) 2022 Richard Knight


    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program; if not, write to the Free Software Foundation,
    Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 
 */

#include <plugin.h>

STRINGDAT* arrayInit(csnd::Csound* csound, ARRAYDAT* array, int rows, int cols) {
    int totalResults = rows * cols;
    size_t totalAllocated;
    
    if (array->data == NULL) {
        array->sizes = (int32_t*) csound->calloc(sizeof(int32_t) * 2);
        array->sizes[0] = rows;
        array->sizes[1] = cols;
        array->dimensions = 2;
        CS_VARIABLE *var = array->arrayType->createVariable(csound->get_csound(), NULL);
        array->arrayMemberSize = var->memBlockSize;
        totalAllocated = array->arrayMemberSize * totalResults;
        array->data = (MYFLT*) csound->calloc(totalAllocated);
    } else if ((totalAllocated = array->arrayMemberSize * totalResults) > array->allocated) {
        array->data = (MYFLT*) csound->realloc(array->data, totalAllocated);
        memset((char*)(array->data)+array->allocated, '\0', totalAllocated - array->allocated);
        array->allocated = totalAllocated;
    }
    
    // convenience return to be used if it is a string array
    return (STRINGDAT*) array->data;
}


void insertArrayStringItem(csnd::Csound* csound, STRINGDAT* strings, int index, char* item) {
    strings[index].size = strlen(item) + 1;
    if (strings[index].data != NULL) {
        csound->free(strings[index].data);
    }
    strings[index].data = csound->strdup(item);
}