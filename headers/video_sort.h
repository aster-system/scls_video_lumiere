#ifndef VIDEO_SORT
#define VIDEO_SORT

#include "../video.h"
#include "../../PLEOS/pleos_libs/pleos_it.h"
#include "../../PLEOS/pleos_libs/pleos_mathematics.h"

//******************
//
// Sort
//
//******************

void bubble_video(std::string path, int datas_number, int sort_width, int width, int height);
void fusion_video(std::string path, int datas_number, int sort_width, int width, int height);
void insertion_video(std::string path, int datas_number, int sort_width, int width, int height);
void selection_video(std::string path, int datas_number, int sort_width, int width, int height);
void sort_all();

#endif // VIDEO_SORT
