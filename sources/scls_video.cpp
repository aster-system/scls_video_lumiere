//******************
//
// scls_video.cpp
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
// This file contains the source code of scls_video.h.
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

    // Convert RGB to YCbCr
    void rgb_to_ycbcr(double& red, double& green, double& blue) {
        // Calculate the values
        double y = 0.299 * red + 0.587 * green + 0.114 * blue;
        double Cb = -0.1687 * red - 0.3313 * green + 0.5 * blue + 128.0;
        double Cr = 0.5 * red - 0.4187 * green - 0.0813 * blue + 128.0;

        // Set the values
        red = y;
        green = Cb;
        blue = Cr;
    }

    //******************
    //
    // Video_Decoder class
    //
    //******************

    // Closes the decoder
    void Video_Decoder::close_decoder() {
        // Reset the audio stream
        if(a_audio_stream.get() != 0) {
            av_frame_free(&a_audio_stream.get()->frame);av_frame_free(&a_audio_stream.get()->temp_frame);
            av_packet_free(&a_audio_stream.get()->packet);
            avcodec_free_context(&a_audio_stream.get()->context);
            a_audio_stream.reset();
        }

        // Reset the video stream
        if(a_video_stream.get() != 0) {
            av_frame_free(&a_video_stream.get()->frame);av_frame_free(&a_video_stream.get()->temp_frame);
            av_packet_free(&a_video_stream.get()->packet);
            avcodec_free_context(&a_video_stream.get()->context);
            a_video_stream.reset();
        }

        // Reset the context
        avformat_close_input(&a_format_context);
    }

    // Decodes a frame
    bool Video_Decoder::decode_frame(){
        // Get the packet
        int result = av_read_frame(a_format_context, a_video_stream.get()->packet);
        if(result < 0){return false;}
        else {decode_packet(a_video_stream.get()->packet);}//*/

        a_current_frame++;
        return true;
    }

    // Decodes a packet
    int Video_Decoder::decode_packet(AVPacket* packet){
        if(a_audio_stream.get() != 0 && packet->stream_index == a_audio_stream.get()->stream_index){a_audio_stream.get()->packet = packet;return decode_packet(a_audio_stream.get());}
        a_video_stream.get()->packet = packet;return decode_packet(a_video_stream.get());;
    }
    int Video_Decoder::decode_packet(Stream* current_stream) {
        // Handle the packet
        int result = avcodec_send_packet(current_stream->context, current_stream->packet);
        if (result < 0) {scls::print("Video decoder", std::string("Can't send the packet to the decoder : error ") + std::to_string(result) + std::string("."));return 1;}

        if(a_current_frame > 0){av_frame_unref(current_stream->temp_frame);}
        while (result >= 0) {
            // Decode the packet
            result = avcodec_receive_frame(current_stream->context, current_stream->frame);
            if (result == AVERROR(EAGAIN) || result == AVERROR_EOF) {break;}
            else if (result < 0) {scls::print("Video decoder", std::string("Can't receive the frame from the decoder : error ") + std::to_string(result) + std::string("."));return 1;};
            current_stream->frame->pts = current_stream->frame->best_effort_timestamp;
            current_stream->temp_frame->pts = current_stream->temp_frame->best_effort_timestamp;

            // Handle the frame
            av_frame_ref(current_stream->temp_frame, current_stream->frame);
            av_frame_unref(current_stream->frame);
        }

        // Finish the decoding
        if(current_stream == a_audio_stream.get()){a_current_frame_is_audio = true;}else{a_current_frame_is_audio = false;}
        scls::print("Video decoder", std::string("Decoding ") + std::to_string(a_current_frame + 1));
        return 0;
    }

    // Opens the decoder
    int Video_Decoder::open_decoder(std::string path) {
        // Open the file
        const char* filename = path.c_str();
        int result = avformat_open_input(&a_format_context, filename, NULL, NULL);
        if (result < 0) {scls::print("Video decoder", std::string("Cannot open the file \"") + path + std::string("\"."));return result;}
        // Get datas about the file
        result = avformat_find_stream_info(a_format_context, NULL);
        if (result < 0) {scls::print("Video decoder", "Cannot find the stream informations");return result;}

        // Audio stream
        result = open_decoder_audio();

        //******************
        // Video stream
        //******************

        // Creates the video stream
        a_video_stream = std::make_shared<Stream>();

        // Find the good stream in the file
        result = av_find_best_stream(a_format_context, AVMEDIA_TYPE_VIDEO, -1, -1, &a_video_stream.get()->codec, 0);
        if (result < 0) {scls::print("Video decoder", "Cannot find a video stream in the input file.");return result;}
        a_video_stream.get()->stream = a_format_context->streams[result];
        a_video_stream.get()->stream_index = result;

        // Create the decoding context
        a_video_stream.get()->context = avcodec_alloc_context3(a_video_stream.get()->codec);
        if (a_video_stream.get()->context == 0){return AVERROR(ENOMEM);}
        avcodec_parameters_to_context(a_video_stream.get()->context, a_video_stream.get()->stream->codecpar);
        // Init the decoder
        result = avcodec_open2(a_video_stream.get()->context, a_video_stream.get()->codec, NULL);
        if (result < 0) {scls::print("Video decoder", "Cannot open the video decoder.");return result;}

        // Create the frame
        a_video_stream.get()->frame = av_frame_alloc();
        a_video_stream.get()->temp_frame = av_frame_alloc();
        if(a_video_stream.get()->temp_frame == 0 || a_video_stream.get()->frame == 0) {scls::print("Video decoder", "Cannot create a frame.");return 1;}
        // Create a packet
        a_video_stream.get()->packet = av_packet_alloc();
        if (a_video_stream.get()->packet == 0) {scls::print("Video decoder", "Cannot create a packet.");return 1;}

        return 0;
    }
    int Video_Decoder::open_decoder_audio() {
        // Creates the audio stream
        int result = 0;
        a_audio_stream = std::make_shared<Stream>();

        // Find the good stream in the file
        result = av_find_best_stream(a_format_context, AVMEDIA_TYPE_AUDIO, -1, -1, &a_audio_stream.get()->codec, 0);
        if (result < 0) {scls::print("Video decoder", "Cannot find a audio stream in the input file.");return result;}
        a_audio_stream.get()->stream = a_format_context->streams[result];
        a_audio_stream.get()->stream_index = result;

        // Create the decoding context
        a_audio_stream.get()->context = avcodec_alloc_context3(a_audio_stream.get()->codec);
        if (a_audio_stream.get()->context == 0){return AVERROR(ENOMEM);}
        avcodec_parameters_to_context(a_audio_stream.get()->context, a_audio_stream.get()->stream->codecpar);
        // Init the parser
        a_audio_stream.get()->parser = av_parser_init(a_audio_stream.get()->codec->id);
        // Init the decoder
        result = avcodec_open2(a_audio_stream.get()->context, a_audio_stream.get()->codec, NULL);
        if (result < 0) {scls::print("Video decoder", "Cannot open the audio decoder.");return result;}

        // Create the frame
        a_audio_stream.get()->frame = av_frame_alloc();
        a_audio_stream.get()->temp_frame = av_frame_alloc();
        if(a_audio_stream.get()->temp_frame == 0 || a_audio_stream.get()->frame == 0) {scls::print("Video decoder", "Cannot create a frame.");return 1;}
        // Create a packet
        a_audio_stream.get()->packet = av_packet_alloc();
        if (a_audio_stream.get()->packet == 0) {scls::print("Video decoder", "Cannot create a packet.");return 1;}//*/

        // Returns the result
        return result;
    }
}
