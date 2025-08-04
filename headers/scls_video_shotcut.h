//******************
//
// scls_video_shotcut.h
//
//******************
// Presentation :
//
// SCLS is a project containing base functions for C++.
// It can also be use in any projects.
//
// The Video "Lumiere" part is usefull to handle video with SCLS.
// It is named after ones of the mainest cinema developers, the brother Louis and Auguste Lumiere.
//
// This file some tools to use the editing software shotcut.
//
//******************
//
// License (LGPL V3.0) :
//
// Copyright (C) 2024 by Aster System, Inc. <https://aster-system.github.io/aster-system/>
// This file is part of SCLS.
// SCLS is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
// SCLS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
// You should have received a copy of the GNU General Public License along with SCLS. If not, see <https://www.gnu.org/licenses/>.
//

#include "../video.h"

// The namespace "scls" is used to simplify the all.
namespace scls {

    //******************
    //
    // Basic video features
    //
    //******************

    // Auto-cuts a video path
    void shotcut_auto_cut(std::string project_path, std::string video_path);
}
