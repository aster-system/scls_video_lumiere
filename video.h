#ifndef SCLS_VIDEO
#define SCLS_VIDEO

extern "C" {
    #include "video_base.h"
}

#include "../scls-graphic-benoit/scls_graphic.h"

// The namespace "scls" is used to simplify the all.
namespace scls {

    //******************
    //
    // Basic video features
    //
    //******************

    // Convert RGB to YCbCr
    void rgb_to_ycbcr(double& red, double& green, double& blue);

    //******************
    //
    // Video_Decoder class
    //
    //******************

    // Struct representating a stream of data
    struct Stream {
        // Needed coded
        const AVCodec* codec;
        // Context for the stream
        AVCodecContext* context = 0;
        // Needed frame for the stream
        AVFrame *frame = 0;
        // Packet for the stream
        AVPacket *packet = 0;
        // Parser for the stream
        AVCodecParserContext *parser = 0;
        // Stream
        AVStream *stream = 0;
        int stream_index = 0;
        // (Sometimes) needed temporary
        AVFrame *temp_frame = 0;

        // Datas for the audio encoding
        double samples_count = 0;double t = 0;double tincr = 0;double tincr2 = 0;
        // Resampling context
        struct SwsContext *sws_ctx;
        struct SwrContext *swr_ctx;
    };

    class Video_Decoder {
        // Class representating a video decoder
    public:

        // Video_Decoder constructor
        Video_Decoder(std::string path){open_decoder(path);}

        // Closes the decoder
        void close_decoder();
        // Decodes a frame
        bool decode_frame();
        // Decodes a packet
        int decode_packet(AVPacket* packet);
        int decode_packet(Stream* current_stream);
        // Opens the decoder
        int open_decoder(std::string path);
        int open_decoder_audio();

        // Getters and setters
        inline int audio_codec_id()const{return a_audio_stream.get()->codec->id;};
        inline int audio_sample_format(){return a_audio_stream.get()->context->sample_fmt;}
        inline std::string context_name()const{return a_format_context->iformat->long_name;};
        inline int current_audio_frame_samples(){if(a_audio_stream.get() == 0){return 0;}return a_audio_stream.get()->temp_frame->nb_samples;};
        inline AVPacket* current_audio_packet(){if(a_audio_stream.get() == 0){return 0;}return a_audio_stream.get()->packet;};
        inline AVFrame* current_audio_frame_stream(){if(a_audio_stream.get() == 0){return 0;}return a_audio_stream.get()->temp_frame;};
        inline bool current_frame_is_audio() const {return a_current_frame_is_audio;};
        inline AVFrame* current_frame_stream(){return a_video_stream.get()->temp_frame;};
        inline int current_audio_frame_channel_number(){if(a_audio_stream.get() == 0){return 0;}return a_audio_stream.get()->context->ch_layout.nb_channels;};
        inline double current_audio_frame_duration(){if(a_audio_stream.get() == 0){return 0;}return static_cast<double>(a_audio_stream.get()->temp_frame->pts) * (static_cast<double>(a_audio_stream.get()->context->time_base.num) / static_cast<double>(a_audio_stream.get()->context->time_base.den));};
        inline int duration(){return a_format_context->duration / AV_TIME_BASE;};
    private:
        // Current decoded frame
        int a_current_frame = 0;bool a_current_frame_is_audio = false;

        // Needed format
        AVFormatContext* a_format_context = 0;

        // Needed streams
        std::shared_ptr<Stream> a_audio_stream;
        std::shared_ptr<Stream> a_video_stream;
    };

    //******************
    //
    // Video_Encoder class
    //
    //******************

    class Video_Encoder {
        // Class representating a video encoder
    public:

        // Video_Encoder constructor
        Video_Encoder(std::string path, double duration, int width, int height):a_duration(duration){
            // Datas for the video
            a_frames_count = duration * static_cast<double>(a_frame_rate);
            a_width = width; a_height = height;

            // Allocate the "AVFormat"
            const char* path_to_ca = path.c_str();
            avformat_alloc_output_context2(&a_format_context, NULL, NULL, path_to_ca);
            if(!a_format_context){
                scls::print("Video", "Could not get an output format from file extension, testing with MPEG.");
                avformat_alloc_output_context2(&a_format_context, NULL, "mpeg", path_to_ca);
            }
            if(!a_format_context){scls::print("Video", "Could not get an output format for this file.");}

            // Check the video and audio codec
            if(video_codec_id() != AV_CODEC_ID_NONE) {create_stream(a_video_stream, &a_video_codec, video_codec_id());}
            if(audio_codec_id() != AV_CODEC_ID_NONE) {create_stream(a_audio_stream, &a_audio_codec, audio_codec_id());}

            // Open the codec
            AVDictionary *opt = 0;bool use_audio=true;bool use_video = true;
            if(use_audio){open_audio(a_audio_stream.get(), opt);}
            if(use_video){open_video(a_video_stream.get(), opt);}
            av_dump_format(a_format_context, 0, path_to_ca, 1);

            // Open the output file (if needed)
            if (!(a_format_context->oformat->flags & AVFMT_NOFILE)) {
                int result = avio_open(&a_format_context->pb, path_to_ca, AVIO_FLAG_WRITE);
                if (result < 0){scls::print("Video", std::string("Could not open the file \"") + path + std::string("\"."));}
            }
            // Write the header (if needed)
            int result = avformat_write_header(a_format_context, &opt);
            if(result < 0){scls::print("Video", std::string("Could not write the header of the file."));}
        }

        // Allocates a frame for the audio
        AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, const AVChannelLayout *channel_layout, int sample_rate, int nb_samples){
            // Create the frame
            AVFrame *frame = av_frame_alloc();
            if (frame == 0) {return 0;}
            frame->format = sample_fmt;
            av_channel_layout_copy(&frame->ch_layout, channel_layout);
            frame->sample_rate = sample_rate;
            frame->nb_samples = nb_samples;

            // Create a buffer for the frame
            if(nb_samples > 0) {if (av_frame_get_buffer(frame, 0) < 0) {scls::print("Video", "Could not create a buffer for the audio frame.");return 0;}}

            return frame;
        }
        // Allocates a frame for the video
        AVFrame *alloc_video_frame(enum AVPixelFormat pix_fmt, int width, int height) {
            // Create the frame
            AVFrame* frame = av_frame_alloc();
            if (frame == 0){return 0;}
            frame->format = pix_fmt;
            frame->width  = width;
            frame->height = height;

            // Create a buffer for the frame
            if (av_frame_get_buffer(frame, 0) < 0) {scls::print("Video", "Could not create a buffer for the video frame.");return 0;}

            return frame;
        }
        // Closes the encoding of the video
        void close_encoding() {
            // Finish the encoding
            av_write_trailer(a_format_context);

            // Close each codecs
            bool use_video = true; bool use_audio = true;
            if(use_audio){close_stream(a_audio_stream);}
            if(use_video){close_stream(a_video_stream);}
            // Close the file
            if (!(a_format_context->oformat->flags & AVFMT_NOFILE)){avio_closep(&a_format_context->pb);}

            // Delete the stream
            avformat_free_context(a_format_context);
        }
        // Closes a stream
        void close_stream(std::shared_ptr<Stream>& current_stream_shared_ptr) {
            Stream* current_stream = current_stream_shared_ptr.get();
            avcodec_free_context(&current_stream->context);
            av_frame_free(&current_stream->frame);
            av_frame_free(&current_stream->temp_frame);
            av_packet_free(&current_stream->packet);
            sws_freeContext(current_stream->sws_ctx);
            swr_free(&current_stream->swr_ctx);
            current_stream = 0;
            current_stream_shared_ptr.reset();
        }
        // Creates a stream
        int create_stream(std::shared_ptr<Stream>& current_stream_shared_ptr, const AVCodec **codec, enum AVCodecID codec_id){
            current_stream_shared_ptr = std::make_shared<Stream>();
            Stream* current_stream = current_stream_shared_ptr.get();
            int needed_bit_rate = 179000;int needed_sample_rate = 48000;

            // Find the good encoder
            *codec = avcodec_find_encoder(codec_id);
            if(!(*codec)){scls::print("Video", std::string("Could not find the encoder : ") + std::string(avcodec_get_name(codec_id)) + std::string("."));return 1;}

            // Allocate the packet
            current_stream->packet = av_packet_alloc();
            if (current_stream->packet  == 0) {scls::print("Video", std::string("Could not allocate a packet."));return 1;}
            // Allocate the stream
            current_stream->stream = avformat_new_stream(a_format_context, NULL);
            if(current_stream->stream == 0) {scls::print("Video", std::string("Could not allocate a stream."));return 1;}
            current_stream->stream->id = a_format_context->nb_streams-1;
            // Allocate the context
            AVCodecContext* stream_context = avcodec_alloc_context3(*codec);
            if (stream_context  == 0) {scls::print("Video", std::string("Could not allocate a context for encoding."));return 1;}
            current_stream->context = stream_context;

            // Handle the precise datas for the context
            if((*codec)->type == AVMEDIA_TYPE_AUDIO) {
                // Codec audio
                stream_context->sample_fmt  = (*codec)->sample_fmts ?(*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                stream_context->bit_rate = needed_bit_rate;
                stream_context->sample_rate = needed_sample_rate;
                if ((*codec)->supported_samplerates) {
                    stream_context->sample_rate = (*codec)->supported_samplerates[0];
                    for (int i = 0; (*codec)->supported_samplerates[i]; i++) {
                        if ((*codec)->supported_samplerates[i] == needed_sample_rate){stream_context->sample_rate = needed_sample_rate;}
                    }
                }
                AVChannelLayout needed_channel = (AVChannelLayout)AV_CHANNEL_LAYOUT_STEREO;
                av_channel_layout_copy(&stream_context->ch_layout, &needed_channel);
            }
            else if((*codec)->type == AVMEDIA_TYPE_VIDEO) {
                // Coded video
                stream_context->codec_id = codec_id;

                stream_context->bit_rate = 400000;
                /* Resolution must be a multiple of two. */
                stream_context->width    = a_width;
                stream_context->height   = a_height;
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                 * of which frame timestamps are represented. For fixed-fps content,
                 * timebase should be 1/framerate and timestamp increments should be
                 * identical to 1. */
                 AVRational needed_time_base = AVRational();needed_time_base.num = 1;needed_time_base.den = a_frame_rate;
                stream_context->time_base = needed_time_base;

                stream_context->gop_size      = 12; /* emit one intra frame every twelve frames at most */
                stream_context->pix_fmt       = STREAM_PIX_FMT;
                if (stream_context->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
                    /* just for testing, we also add B-frames */
                    stream_context->max_b_frames = 2;
                }
                if (stream_context->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
                    /* Needed to avoid using macroblocks in which some coeffs overflow.
                     * This does not happen with normal video, it just happens here as
                     * the motion of the chroma plane does not match the luma plane. */
                    stream_context->mb_decision = 2;
                }
            }

            // Separate stream header for some formats
            if(a_format_context->oformat->flags & AVFMT_GLOBALHEADER){stream_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;}
            current_stream->codec = *codec;
            return 0;
        };
        // Returns the current video frame
        AVFrame *current_video_frame(Stream* current_stream) {
            // Check if a little duration must be waited
            //AVRational one;one.num = 1;one.den = 1;
            //if(av_compare_ts(a_current_frame, current_stream->context->time_base, static_cast<double>(a_frames_count) / static_cast<double>(a_frame_rate), one) > 0){return 0;}

            // Generate the image from an scls::Image
            if(a_current_frame_type == 0) {
                // Make frame writable
                if(av_frame_make_writable(current_stream->frame) < 0){return 0;}

                // Check the good image format
                if (current_stream->context->pix_fmt != AV_PIX_FMT_YUV420P) {
                    // Convert frame to YUV420P
                    if (!current_stream->sws_ctx) {
                        current_stream->sws_ctx = sws_getContext(current_stream->context->width, current_stream->context->height,AV_PIX_FMT_YUV420P,current_stream->context->width, current_stream->context->height,current_stream->context->pix_fmt,SCALE_FLAGS, NULL, NULL, NULL);
                        if (!current_stream->sws_ctx) {scls::print("Video", "Could not initialize the conversion context.");return 0;}
                    }
                    fill_yuv_image(current_stream->temp_frame, a_current_frame, current_stream->context->width, current_stream->context->height);
                    sws_scale(current_stream->sws_ctx, (const uint8_t * const *) current_stream->temp_frame->data,current_stream->temp_frame->linesize, 0, current_stream->context->height, current_stream->frame->data,current_stream->frame->linesize);
                }
                else {fill_yuv_image(current_stream->frame, a_current_frame, current_stream->context->width, current_stream->context->height);}
            }

            // Return the result
            return current_stream->frame;
        }
        // Opens the audio encoder
        int open_audio(Stream* current_stream, AVDictionary *opt_arg) {
            int ret;

            // Open the coded
            AVDictionary *opt = 0;
            av_dict_copy(&opt, opt_arg, 0);
            int result = avcodec_open2(current_stream->context, current_stream->codec, &opt);
            av_dict_free(&opt);if (result < 0) {scls::print("Video", std::string("Could not open the audio codec."));return 1;}

            // Open the signal generator
            current_stream->t = 0;
            current_stream->tincr = 2.0 * SCLS_PI * 110.0 / current_stream->context->sample_rate;
            current_stream->tincr2 = 2.0 * SCLS_PI * 110.0 / current_stream->context->sample_rate / current_stream->context->sample_rate;
            int nb_samples = 0;
            if (current_stream->context->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE){nb_samples = 10000;}
            else{nb_samples = current_stream->context->frame_size;}

            // Allocate the frame
            current_stream->frame = alloc_audio_frame(current_stream->context->sample_fmt, &current_stream->context->ch_layout, current_stream->context->sample_rate, nb_samples);
            current_stream->temp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, &current_stream->context->ch_layout, current_stream->context->sample_rate, nb_samples);

            // Copy the parameters in the muxer
            result = avcodec_parameters_from_context(current_stream->stream->codecpar, current_stream->context);
            if (result < 0) {scls::print("Video", std::string("Could not copy the parameters of the stream."));return 1;}

            // Create the resampler
            current_stream->swr_ctx = swr_alloc();
            if (!current_stream->swr_ctx) {scls::print("Video", std::string("Could not create the resampler."));return 1;}

            // Set the needed options
            av_opt_set_chlayout(current_stream->swr_ctx, "in_chlayout", &current_stream->context->ch_layout, 0);
            av_opt_set_int(current_stream->swr_ctx, "in_sample_rate", current_stream->context->sample_rate, 0);
            av_opt_set_sample_fmt(current_stream->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
            av_opt_set_chlayout(current_stream->swr_ctx, "out_chlayout", &current_stream->context->ch_layout, 0);
            av_opt_set_int(current_stream->swr_ctx, "out_sample_rate", current_stream->context->sample_rate, 0);
            av_opt_set_sample_fmt(current_stream->swr_ctx, "out_sample_fmt", current_stream->context->sample_fmt, 0);

            // Init the resampler
            if ((ret = swr_init(current_stream->swr_ctx)) < 0) {scls::print("Video", std::string("Could not initialize the resampler."));return 1;}

            return 0;
        }
        // Opens the video encoder
        int open_video(Stream* current_stream, AVDictionary *opt_arg) {
            // Get the datas for the codec
            const AVCodec *codec = current_stream->codec;
            AVCodecContext *current_context = current_stream->context;
            AVDictionary *settings = 0;
            av_dict_copy(&settings, opt_arg, 0);

            // Open the codec
            int result = avcodec_open2(current_context, codec, &settings);
            av_dict_free(&settings);
            if (result < 0) {scls::print("Video", std::string("Can't open the codec."));return 1;}

            // Allocates a frame for the stream
            current_stream->frame = alloc_video_frame(current_context->pix_fmt, current_context->width, current_context->height);
            if(current_stream->frame == 0) {scls::print("Video", std::string("Could not allocate a frame."));return 1;}

            // If the output format is not YUV420P
            if (current_context->pix_fmt != AV_PIX_FMT_YUV420P) {
                current_stream->temp_frame = alloc_video_frame(AV_PIX_FMT_YUV420P, current_context->width, current_context->height);
                if (current_stream->temp_frame == 0) {scls::print("Video", std::string("Could not allocate a temporary frame."));return 1;}
            }

            /* copy the stream parameters to the muxer */
            result = avcodec_parameters_from_context(current_stream->stream->codecpar, current_context);
            if (result < 0) {scls::print("Video", std::string("Could not copy the needed parameters."));return 1;}

            return 0;
        }
        // Write a frame to the file
        int write_frame(AVPacket* packet) {
            // Write the frame
            scls::print("Video", std::string("Encoding : frame ") + std::to_string(a_current_frame + 1) + std::string(" / ") + std::to_string(a_frames_count));
            int result = av_interleaved_write_frame(a_format_context, packet);
            if (result < 0) {scls::print("Video", "Could not write the frame.");return 1;}
            return result;
        }
        int write_frame(Stream* current_stream) {
            // Send the frame to the encoder
            if(current_stream == a_video_stream.get()){current_video_frame(current_stream);}
            int result = avcodec_send_frame(current_stream->context, current_stream->frame);
            if(result < 0){scls::print("Video", "Could not send the frame to the encoder.");return 1;}

            // Compress the frame
            while (result >= 0) {
                result = avcodec_receive_packet(current_stream->context, current_stream->packet);
                if (result == AVERROR(EAGAIN) || result == AVERROR_EOF){break;}
                else if (result < 0) {scls::print("Video", std::string("Could not encode the frame : error ") + std::to_string(result) + std::string("."));return 1;}

                // Rescale the frame
                av_packet_rescale_ts(current_stream->packet, current_stream->context->time_base, current_stream->stream->time_base);
                current_stream->packet->stream_index = current_stream->stream->index;

                // Write the frame
                result = write_frame(current_stream->packet);
            }

            return result == AVERROR_EOF ? 1 : 0;
        }

        // Write a video frame
        inline int write_video_frame(AVFrame* image){a_current_frame_type=1;av_frame_make_writable(a_video_stream.get()->frame);av_frame_copy(a_video_stream.get()->frame, image);return write_frame(a_video_stream.get());};
        inline int write_video_frame(std::shared_ptr<scls::Image> image){a_current_frame_type=0;a_current_image = image;return write_frame(a_video_stream.get());};
        inline int write_video_frame(){return write_video_frame(a_current_image);};

        // Prepare the frame
        void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height) {
            // Create an image
            if(a_current_image.get() == 0) {a_current_image = std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255));}
            else{
                if(a_current_image.get()->width() != width && a_current_image.get()->height() != height) {
                   scls::print("Video", std::string("A provided image of size (") + std::to_string(a_current_image.get()->width()) + std::string(", ") + std::to_string(a_current_image.get()->height()) + std::string(" has not the same size of the video (") + std::to_string(width) + std::string(", ") + std::to_string(height) + std::string(")."));
                   a_current_image = std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255));
                }
            }

            // Y
            for (int y = 0; y < height; y++){
                for (int x = 0; x < width; x++){
                    scls::Color current_color = a_current_image.get()->pixel_directly((x + y * width) * a_current_image.get()->components(), 1);
                    double red = current_color.red(); double green = current_color.green(); double blue = current_color.blue();
                    rgb_to_ycbcr(red, green, blue);
                    pict->data[0][y * pict->linesize[0] + x] = red;
                }
            }
            // Cb and Cr
            for (int y = 0; y < height/2; y++){
                for (int x = 0; x < width/2; x++){
                    scls::Color current_color = a_current_image.get()->pixel_directly(((x + y * width) * a_current_image.get()->components()) * 2, 1);
                    double red = current_color.red(); double green = current_color.green(); double blue = current_color.blue();
                    rgb_to_ycbcr(red, green, blue);
                    pict->data[1][y * pict->linesize[1] + x] = green;
                    pict->data[2][y * pict->linesize[2] + x] = blue;
                }
            }
        }

        // Returns an audio
        AVFrame* audio_frame(Stream* current_stream) {
            // Get the good frame
            AVFrame *frame = current_stream->temp_frame;
            int16_t *q = reinterpret_cast<int16_t*>(frame->extended_data[0]);

            // Generate the audio
            double multiplier = 32767;
            double t = (static_cast<double>(current_frame()) / static_cast<double>(frame_count()));
            for (int j = 0; j <frame->nb_samples; j++) {
                int16_t v = multiplier * t;
                //v=(std::sin((static_cast<double>(j)  * 3.1415) / 260.0) * multiplier);
                for (int i = 0; i < current_stream->context->ch_layout.nb_channels; i++){*q = v;q++;}
            }

            // Returns the result
            frame->pts = current_frame();
            return frame;
        }
        // Write audio frame
        int __write_audio_frame(AVFrame *frame) {
            AVCodecContext *c = a_audio_stream.get()->context;
            int result = 0;
            int dst_nb_samples;

            if (frame) {
                /* convert samples from native format to destination codec format, using the resampler */
                /* compute destination number of samples */
                dst_nb_samples = swr_get_delay(a_audio_stream.get()->swr_ctx, c->sample_rate) + frame->nb_samples;
                av_assert0(dst_nb_samples == frame->nb_samples);

                /* when we pass a frame to the encoder, it may keep a reference to it
                 * internally;
                 * make sure we do not overwrite it here
                 */
                result = av_frame_make_writable(a_audio_stream.get()->frame);
                if (result < 0)exit(1);

                /* convert to destination format */
                /*result = swr_convert(a_audio_stream.get()->swr_ctx, a_audio_stream.get()->frame->data, dst_nb_samples, (const uint8_t **)frame->data, frame->nb_samples);
                if (result < 0) {
                    fprintf(stderr, "Error while converting\n");
                    exit(1);
                }
                //*/

                frame = a_audio_stream.get()->frame;
                frame->pts = av_rescale_q(a_audio_stream.get()->samples_count, (AVRational){1, c->sample_rate}, c->time_base);
                a_audio_stream.get()->samples_count += dst_nb_samples;
            }

            // Write the frame
            return write_frame(a_audio_stream.get());
        }
        int write_audio_frame(AVFrame *frame){av_frame_copy(a_audio_stream.get()->frame, frame);return __write_audio_frame(a_audio_stream.get()->frame);};
        int write_audio_frame(){return __write_audio_frame(audio_frame(a_audio_stream.get()));}

        // Getters and setters
        inline int audio_channel_number() {return a_audio_stream.get()->context->ch_layout.nb_channels;};
        inline AVCodecID audio_codec_id() {return a_format_context->oformat->audio_codec;}
        inline int audio_sample_format(){return a_audio_stream.get()->context->sample_fmt;}
        inline int audio_sample_rate(){return a_audio_stream.get()->context->sample_rate;}
        inline AVFrame* current_audio_frame_stream() {return a_audio_stream.get()->frame;}
        inline int current_frame()const{return a_current_frame;};
        inline AVFrame* current_frame_stream() {return a_video_stream.get()->frame;}
        inline int frame_count()const{return a_frames_count;};
        inline int height() const {return a_height;};
        inline void go_to_next_frame(){a_current_frame++;};
        inline AVCodecID video_codec_id() {return a_format_context->oformat->video_codec;}
        inline int width() const {return a_width;};
    private:
        // Current frame to generate the video
        int a_current_frame = 0;
        // Currently used frame (0 = scls::Image, 1 = AVFrame)
        int a_current_frame_type = 0;
        // Current image to encode
        std::shared_ptr<scls::Image> a_current_image;
        // Duration of the video in seconds
        double a_duration = 0;
        // Number of frames in the video
        int a_frames_count = 125;
        // Frame rate of the video
        int a_frame_rate = 60;
        // Height of the video
        int a_height = 0;
        // Width of the video
        int a_width = 0;

        // Needed streams
        std::shared_ptr<Stream> a_audio_stream;
        std::shared_ptr<Stream> a_video_stream;

        // Needed format
        AVFormatContext* a_format_context;

        // Basic audio codec
        const AVCodec* a_audio_codec;
        // Basic video codec
        const AVCodec* a_video_codec;
    };

}
#endif // SCLS_VIDEO
