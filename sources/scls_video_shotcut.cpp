//******************
//
// scls_video_shotcut.cpp
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
// This file contains the source file of "scls_video_shotcut.cpp".
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

// Include the needed header
#include "../headers/scls_video_shotcut.h"

// The namespace "scls" is used to simplify the all.
namespace scls {

    //******************
    //
    // Basic video features
    //
    //******************

    // Basics functions
    #define AUDIO_TIME 0.021333333333
    std::shared_ptr<scls::__Balise_Container> mlt_balises() {
        std::shared_ptr<scls::__Balise_Container> balises = std::make_shared<scls::__Balise_Container>();
        std::shared_ptr<scls::Balise_Style_Datas> current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("entry", current_balise);
        current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("mlt", current_balise);
        current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("profile", current_balise);
        current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("playlist", current_balise);
        current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("producer", current_balise);
        current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("property", current_balise);
        current_balise = std::make_shared<scls::Balise_Style_Datas>();
        current_balise.get()->has_content = true;
        balises.get()->set_defined_balise("tractor", current_balise);
        return balises;
    }

    // Auto-cuts a video path
    std::string frame_to_time(double current_frame){int current_minut = 0;while(current_frame > 60.0){current_frame -= 60.0;current_minut++;}return scls::format_number_to_text_strict(current_minut, 2, 0) + std::string(":") + scls::format_number_to_text_strict(current_frame, 2, 3);}
    void shotcut_auto_cut(std::string project_path, std::string video_path) {
        // Create the output video
        int duration = 60;int width = 1920;int height = 1080;
        double current_time = 0;double last_frame = 0;double last_frame_silent = 0;
        std::string total_chain_content = std::string("");
        std::string current_chain = std::string();std::string total_chain = std::string();int current_chain_index = 0;
        std::string mlt_first_part = scls::replace(scls::replace(scls::read_file("assets/shotcut_datas/mlt_1_file.txt"), "WIDTH", std::to_string(width)), "HEIGHT", std::to_string(height));
        std::string mlt_second_part = scls::read_file("assets/shotcut_datas/mlt_2_file.txt");
        std::string mlt_third_part = scls::read_file("assets/shotcut_datas/mlt_3_file.txt");

        // Create the input video
        scls::Video_Decoder video_decoder = scls::Video_Decoder(video_path);

        // Decode the target
        bool error_occured = false;
        int current_frame = 0;double minimum_chain_time = 0.2;double minimum_silence_time = 0.1;double volume_action = 0.5;
        while(video_decoder.decode_frame()){
            if(!video_decoder.current_frame_is_audio()){continue;}

            float *datas = reinterpret_cast<float*>(video_decoder.current_audio_frame_stream()->extended_data[0]);
            // Average sample
            float sample_max = 0;double sample_average = 0;
            if(video_decoder.current_audio_frame_stream() != 0 && video_decoder.current_audio_frame_samples() > 0) {
                double total_sample = 0;
                int j = 0;for(;j<video_decoder.current_audio_frame_samples();j++) {
                    if(std::abs(static_cast<float>(*datas))>std::abs(sample_max)){sample_max = static_cast<float>(*datas);}
                    total_sample += static_cast<float>(*datas);datas++;datas++;
                }
                sample_average = total_sample / static_cast<double>(j);
            }

            // Draw the part
            sample_average *= 500.0;sample_max *= 250.0;

            // Handle the balises
            if(current_chain == std::string()) {
                if(std::abs(sample_max) > volume_action) {
                    current_chain = std::string("    <entry producer=\"chain") + std::to_string(current_chain_index) + std::string("\" in=\"00:00:") + frame_to_time(current_time - 0.067) + std::string("\"");
                    last_frame = current_time;last_frame_silent=0;
                }
            }
            else {
                if(std::abs(sample_max) < volume_action) {
                    if(last_frame_silent > minimum_silence_time) {
                        if(current_time - last_frame > minimum_chain_time) {
                            current_chain += std::string(" out=\"00:") + frame_to_time(current_time) + std::string("\"/>");
                            total_chain += current_chain + std::string("\n");
                            total_chain_content += std::string("  <chain id=\"chain") + std::to_string(current_chain_index) + std::string("\"><property name=\"resource\">") + video_path + std::string("</property></chain>\n");
                            current_chain_index++;
                        }

                        current_chain = std::string();
                    }

                    last_frame_silent += AUDIO_TIME;
                }
                else{last_frame_silent=0;}
            }

            current_time += AUDIO_TIME;current_frame++;

            // Asserts
            if(current_frame > 1000 && total_chain_content == std::string()) {scls::print("SCLS Video", "Error");error_occured=true;break;}
        }

        scls::write_in_file(project_path, mlt_first_part + total_chain_content + mlt_second_part + total_chain + mlt_third_part);

        if(error_occured){shotcut_auto_cut(project_path, video_path);}
    }
}
