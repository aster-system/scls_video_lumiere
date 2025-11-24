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
    // Basic audio features
    //
    //******************

    // Returns the name of an object
    std::string codec_name_by_id(AVCodecID codec_id);
    std::string sample_format_name(enum AVSampleFormat sample_format_id);
    // Returns empty audio datas
    std::shared_ptr<scls::Bytes_Set> empty_audio_datas();
    // Returns full audio datas from a file
    std::vector<std::shared_ptr<scls::Bytes_Set>> load_audio_datas_samples(std::string path);

    // Add datas to an audio sample
    void add_audio_datas(std::shared_ptr<scls::Bytes_Set> basic_datas, std::shared_ptr<scls::Bytes_Set> datas_to_add, int divisor);

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

        // Current frame to generate the stream
        int current_frame = 0;
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
        Video_Decoder(std::string path);

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
        inline std::string audio_codec_name()const{return codec_name_by_id(a_audio_stream.get()->codec->id);};
        inline int audio_sample_format(){return a_audio_stream.get()->context->sample_fmt;}
        inline std::string audio_sample_format_name(){return sample_format_name(a_audio_stream.get()->context->sample_fmt);}
        inline int audio_sample_number(){return a_audio_stream.get()->frame->nb_samples;}
        inline int audio_sample_rate(){return a_audio_stream.get()->context->sample_rate;}
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
        // Packet to decode
        AVPacket *a_packet_to_decode = 0;

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

        // Struct containing an audio track
        struct Audio_Track{
            // Audio_Track constructor
            Audio_Track(std::vector<std::shared_ptr<scls::Bytes_Set>> needed_datas);

            int current_frame = 0;
            std::vector<std::shared_ptr<scls::Bytes_Set>> datas;
        };

        // Video_Encoder constructor
        Video_Encoder(std::string path, double duration, int width, int height);

        // Allocates a frame for the audio
        AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, const AVChannelLayout *channel_layout, int sample_rate, int nb_samples);
        // Allocates a frame for the video
        AVFrame *alloc_video_frame(enum AVPixelFormat pix_fmt, int width, int height);
        // Closes the encoding of the video
        void close_encoding();
        // Closes a stream
        void close_stream(std::shared_ptr<Stream>& current_stream_shared_ptr);
        // Creates a stream
        int create_stream(std::shared_ptr<Stream>& current_stream_shared_ptr, const AVCodec **codec, enum AVCodecID codec_id);
        // Returns the current video frame
        AVFrame *current_video_frame(Stream* current_stream);
        // Opens the audio encoder
        int open_audio(Stream* current_stream, AVDictionary *opt_arg);
        // Opens the video encoder
        int open_video(Stream* current_stream, AVDictionary *opt_arg);
        // Write a frame to the file
        int write_frame(AVPacket* packet);
        int write_frame(Stream* current_stream);

        // Write a video frame
        int write_video_frame(AVFrame* image);
        int write_video_frame(scls::Image image);
        int write_video_frame(std::shared_ptr<scls::__Image_Base> image);
        int write_video_frame();

        // Prepare the frame
        void fill_yuv_image(AVFrame *pict, int frame_index, int width, int height);

        // Returns an audio
        AVFrame* audio_frame(Stream* current_stream);
        // Returns the current audio frame from tracks
        std::shared_ptr<scls::Bytes_Set> audio_frame_tracks();
        // Write audio frame
        int __write_audio_frame(AVFrame *frame);
        int write_audio_frame(std::shared_ptr<scls::Bytes_Set> frame);
        int write_audio_frame(AVFrame *frame);
        int write_audio_frame(float *frame);
        int write_audio_frame();
        // Write audio frame samples
        int write_audio_frame_samples(std::vector<std::shared_ptr<scls::Bytes_Set>>& datas);

        // Getters and setters
        inline int audio_channel_number() {return a_audio_stream.get()->context->ch_layout.nb_channels;};
        inline AVCodecID audio_codec_id() {return a_format_context->oformat->audio_codec;}
        inline std::string audio_codec_name(){return codec_name_by_id(a_format_context->oformat->audio_codec);};
        inline int audio_sample_format(){return a_audio_stream.get()->context->sample_fmt;}
        inline int audio_sample_number(){return a_audio_stream.get()->frame->nb_samples;}
        inline int audio_sample_rate(){return a_audio_stream.get()->context->sample_rate;}
        inline int64_t current_audio_frame_duration() {return a_audio_stream.get()->frame->duration;}
        inline AVFrame* current_audio_frame_stream() {return a_audio_stream.get()->frame;}
        inline int current_frame_video()const{if(a_video_stream.get() == 0){return 0;}return a_video_stream.get()->current_frame;};
        inline AVFrame* current_frame_stream() {return a_video_stream.get()->frame;}
        inline int frame_count()const{return a_frames_count;};
        inline int height() const {return a_height;};
        inline void go_to_frame_video(int needed_frame){if(a_video_stream.get() != 0){a_video_stream.get()->current_frame=needed_frame;};};
        inline void go_to_next_frame_audio(){if(a_audio_stream.get() != 0){a_audio_stream.get()->current_frame++;};};
        inline void go_to_next_frame_video(){if(a_video_stream.get() != 0){a_video_stream.get()->current_frame++;};};
        inline void go_to_next_frame(){go_to_next_frame_audio();go_to_next_frame_video();};
        inline std::vector<Audio_Track>& tracks(){return a_tracks;};
        inline AVCodecID video_codec_id() {return a_format_context->oformat->video_codec;}
        inline int width() const {return a_width;};
    private:
        // Currently used frame (0 = __Image_Base, 1 = AVFrame)
        int a_current_frame_type = 0;
        // Current image to encode
        std::shared_ptr<__Image_Base> a_current_image;
        // Height of the video
        int a_height = 0;
        // Width of the video
        int a_width = 0;

        // Current audio track
        std::vector<Audio_Track> a_tracks;

        // Duration of the video in seconds
        double a_duration = 0;
        // Number of frames in the video
        int a_frames_count = 125;
        // Frame rate of the video
        int a_frame_rate = 60;

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
    typedef Video_Encoder::Audio_Track Audio_Track;

}
#endif // SCLS_VIDEO
