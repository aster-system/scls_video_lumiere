#include "video.h"
#include "../PLEOS/pleos_libs/pleos_it.h"
#include "../PLEOS/pleos_libs/pleos_mathematics.h"

SCLS_INIT

//******************
//
// Tick system
//
//******************

// Tick settings

#define END_TICK first_part_executed = true;
#define PREPARE_TICK_SYSTEM bool first_part_executed = false;
#define START_TICK(needed_start_tick) int current_tick = 0;const int start_tick = needed_start_tick;int tick = video.current_frame() + start_tick;double video_proportion = static_cast<double>(tick) / (duration - 2);
#define TICK_EVENT(start, duration, to_do, to_do_at_end) if(tick >= start && tick < start + duration){double event_proportion=(static_cast<double>(tick) - static_cast<double>(start))/static_cast<double>(duration); to_do}else if(tick == start + duration || (!first_part_executed && start + duration <= start_tick)){to_do_at_end}
#define TICK_CURRENT_EVENT(duration, to_do, to_do_at_end) TICK_EVENT(current_tick, duration, to_do, to_do_at_end);current_tick+=duration;
#define TICK_WAIT(duration) current_tick += duration;

// Tick drawing

#define TICK_FILL_RECT(image, start, duration, x, y, w, h, color, operation) if(tick >= start + duration){image->fill_rect(x, y, w, h, color);}else if(tick >= start){operation}
#define TICK_FILL_RECT_FROM_LEFT(image, start, duration, x, y, w, h, color) TICK_FILL_RECT(image, start, duration, x, y, w, h, color, image->fill_rect(x, y, static_cast<double>(w) * (static_cast<double>(tick - start) / static_cast<double>(duration - 1)), h, color);)
#define TICK_FILL_RECT_FROM_RIGHT(image, start, duration, x, y, w, h, color) TICK_FILL_RECT(image, start, duration, x, y, w, h, color, image->fill_rect(x - static_cast<double>(w) * (static_cast<double>(tick - start) / static_cast<double>(duration - 1)), y, static_cast<double>(w) * (static_cast<double>(tick - start) / static_cast<double>(duration - 1)), h, color);)

class Video_Generator {
    // Class representating a video generator
public:

    // Video_Generator constructor
    Video_Generator(std::string path, int duration_in_tick, int width, int height):a_video(scls::Video_Encoder(path, static_cast<double>(duration_in_tick) / 60.0, width, height)){};

    //******************
    //
    // Image handling
    //
    //******************

    // Fill a rect in the video
    int fill_rect(int start, int duration, int x, int y, int width, int height, scls::Color color){if(tick() >= start + duration){a_current_image.get()->fill_rect(x, y, width, height, color);return 0;}else if(tick() >= start){return 1;}return -1;}
    void fill_rect_from_left(int start, int duration, int x, int y, int width, int height, scls::Color color){if(fill_rect(start, duration, x, y, width, height, color) == 1){double mul=static_cast<double>(a_tick - start) / static_cast<double>(duration);a_current_image.get()->fill_rect(x, y, width * mul, height, color);}}
    void fill_rect_from_right(int start, int duration, int x, int y, int width, int height, scls::Color color){if(fill_rect(start, duration, x, y, width, height, color) == 1){double mul=static_cast<double>(a_tick - start) / static_cast<double>(duration);a_current_image.get()->fill_rect((x + width) - (width * mul), y, width * mul, height, color);}}

    // Starts the frame
    inline void start_frame(int needed_start_tick){a_current_tick=0;a_start_tick=needed_start_tick;a_tick = a_video.current_frame() + a_start_tick;a_video_proportion = static_cast<double>(a_tick) / (frame_count() - 2);};

    // Ticked object
    // Returns if the tick is above / between two values
    inline bool tick_above(int v){return a_tick >= v;};
    inline bool tick_between(int v_1, int v_2){return a_tick >= v_1 && a_tick < v_2;};
    // Returns a ticked point 2d
    scls::Point_2D ticked_point_2d(int start, int duration, scls::Fraction x_start, scls::Fraction y_start, scls::Fraction x_end, scls::Fraction y_end){scls::Fraction mul=scls::Fraction::from_double(static_cast<double>(a_tick - start) / static_cast<double>(duration));return scls::Point_2D(x_start + (x_end - x_start) * mul, y_start + (y_end - y_start) * mul);};
    // Returns a ticked value
    scls::Fraction ticked_value(int start, int duration, scls::Fraction value_start, scls::Fraction value_end)const{return scls::Fraction(value_start) + (value_end - value_start) * scls::Fraction::from_double((static_cast<double>(a_tick) - static_cast<double>(start)) / static_cast<double>(duration));};

    // Getters and setters
    inline scls::Image* current_image()const{return a_current_image.get();};
    inline void set_current_image(std::shared_ptr<scls::Image> new_current_image){a_current_image=new_current_image;};
    inline int tick()const{return a_tick;};

    //******************
    // Video handling
    //******************

    // Video methods
    inline void close_encoding(){a_video.close_encoding();};
    inline void go_to_next_frame(){write_video_frame(a_current_image);a_video.go_to_next_frame();a_first_part_executed = true;};
    inline void write_video_frame(std::shared_ptr<scls::Image> image){a_video.write_video_frame(image);};
    // Getters and setters
    inline int current_frame()const{return a_video.current_frame();};
    inline int frame_count()const{return a_video.frame_count();};

private:
    // Datas about the video
    // Currently used image
    std::shared_ptr<scls::Image> a_current_image;
    // Object to the video
    scls::Video_Encoder a_video;

    // Datas about the tick system
    // Current tick for programmation
    int a_current_tick = 0;
    // Tick to start of the generation
    int a_start_tick;
    // Current tick for generation
    int a_tick = 0;
    // Proportion of the video at this tick
    double a_video_proportion = 0;

    // If the first part has been executed or not
    bool a_first_part_executed = false;
};

//******************
//
// Area
//
//******************

void area_video(std::string path, int width, int height) {
    // Encode the video
    double speed = 1;double duration = 60 * 15;
    std::shared_ptr<pleos::Graphic> graphic = std::make_shared<pleos::Graphic>();
    graphic.get()->set_draw_base(false);graphic.get()->set_draw_sub_bases(false);
    graphic.get()->set_background_color(scls::Color(255, 255, 255));
    graphic.get()->new_point("p1", -3, -1);
    graphic.get()->new_point("p2", 0, -1);
    graphic.get()->new_point("p3", 1, 5);
    graphic.get()->new_point("p5", 1, -1);
    graphic.get()->new_point("p6", -3, 5);

    pleos::Form_2D* f_1 = graphic.get()->new_form("f1", "p1;p2;p3").get();f_1->set_links_drawing_proportion(0);f_1->set_color(scls::Color(0, 255, 0));
    pleos::Form_2D* f_2 = graphic.get()->new_form("f2", "p2;p3;p5").get();f_2->set_links_drawing_proportion(0);f_2->set_color(scls::Color(0, 0, 255, 0));
    pleos::Form_2D* f_3 = graphic.get()->new_form("f3", "p1;p3;p6").get();f_3->set_links_drawing_proportion(0);f_3->set_color(scls::Color(0, 125, 255, 0));

    pleos::Form_2D* l_1 = graphic.get()->new_line("l1", -3, -2, 0, -2).get();l_1->set_links_drawing_proportion(0);
    pleos::Form_2D* l_2 = graphic.get()->new_line("l2", 2, 5, 2, -1).get();l_2->set_links_drawing_proportion(0);
    pleos::Form_2D* l_3 = graphic.get()->new_line("l3", 0, -2, 1, -2).get();l_3->set_links_drawing_proportion(0);l_3->set_border_color(scls::Color(0, 0, 255));
    std::shared_ptr<scls::Text_Style> first_style = std::make_shared<scls::Text_Style>();
    first_style.get()->set_background_color(scls::Color(0, 0, 0, 0));first_style.get()->set_color(scls::Color(0, 0, 0, 0));first_style.get()->set_font_size(100);
    pleos::Graphic::Graphic_Text* t_1 = graphic.get()->new_text("b", scls::Fraction(-3, 2), scls::Fraction(-5, 2), first_style);pleos::Vector t_1_pos = t_1->position();
    pleos::Graphic::Graphic_Text* t_2 = graphic.get()->new_text("h", scls::Fraction(5, 2), 2, first_style);pleos::Vector t_2_pos = t_2->position();
    std::shared_ptr<scls::Text_Style> second_style = std::make_shared<scls::Text_Style>();
    second_style.get()->set_background_color(scls::Color(0, 0, 0, 0));second_style.get()->set_color(scls::Color(0, 0, 0, 0));second_style.get()->set_font_size(90);
    pleos::Graphic::Graphic_Text* t_3 = graphic.get()->new_text("c", scls::Fraction(1, 2), scls::Fraction(-5, 2), second_style);

    // Formula triangles
    std::shared_ptr<scls::Text_Style> third_style = std::make_shared<scls::Text_Style>();
    third_style.get()->set_background_color(scls::Color(0, 0, 0, 0));third_style.get()->set_color(scls::Color(0, 0, 0, 0));third_style.get()->set_font_size(75);
    pleos::Form_2D* ft_1 = graphic.get()->new_triangle("ft1", -3, -4, scls::Fraction(-5, 2), -4, -3, -3).get();ft_1->set_opacity(0);
    pleos::Graphic::Graphic_Text* t_4 = graphic.get()->new_text("+", scls::Fraction(-9, 4), scls::Fraction(-7, 2), third_style);pleos::Vector t_4_pos = t_4->position();
    pleos::Form_2D* ft_2 = graphic.get()->new_triangle("ft2", -2, -4, scls::Fraction(-3, 2), -4, -2, -3).get();ft_2->set_opacity(0);ft_2->set_color(scls::Color(0, 125, 255));
    pleos::Graphic::Graphic_Text* t_5 = graphic.get()->new_text("+", scls::Fraction(-5, 4), scls::Fraction(-7, 2), third_style);pleos::Vector t_5_pos = t_5->position();
    pleos::Form_2D* ft_3 = graphic.get()->new_triangle("ft3", -1, -4, scls::Fraction(-1, 2), -4, -1, -3).get();ft_3->set_opacity(0);ft_3->set_color(scls::Color(0, 0, 255));
    pleos::Graphic::Graphic_Text* t_6 = graphic.get()->new_text("= h * (b + c)", scls::Fraction(3, 2), scls::Fraction(-7, 2), third_style);
    pleos::Graphic::Graphic_Text* t_7 = graphic.get()->new_text("= h * (b + c) / 2", 1, scls::Fraction(-7, 2), third_style);t_7->set_opacity(0);
    pleos::Graphic::Graphic_Text* t_9 = graphic.get()->new_text("= (h * b) / 2", 1, scls::Fraction(-7, 2), third_style);t_9->set_opacity(0);
    std::shared_ptr<scls::Text_Style> third_style_big = std::make_shared<scls::Text_Style>();
    third_style_big.get()->set_background_color(scls::Color(0, 0, 0, 0));third_style_big.get()->set_color(scls::Color(0, 0, 0, 0));third_style_big.get()->set_font_size(60);
    pleos::Graphic::Graphic_Text* t_8 = graphic.get()->new_text("= (h * b + h * c) / 2", 1, scls::Fraction(-7, 2), third_style_big);t_8->set_opacity(0);

    pleos::Form_2D* ft_4 = graphic.get()->new_triangle("ft4", scls::Fraction(-5, 2), -3, -2, -3, scls::Fraction(-5, 2), -2).get();ft_4->set_opacity(0);ft_4->set_color(scls::Color(0, 0, 255));
    pleos::Graphic::Graphic_Text* t_10 = graphic.get()->new_text("= (c * h) / 2", 1, scls::Fraction(-5, 2), third_style);
    pleos::Form_2D* ft_5 = graphic.get()->new_triangle("ft5", scls::Fraction(-5, 2), -2, -2, -2, scls::Fraction(-5, 2), -1).get();ft_5->set_opacity(0);ft_5->set_color(scls::Color(0, 125, 255));
    pleos::Graphic::Graphic_Text* t_11 = graphic.get()->new_text("= h * (b + c) / 2", 1, scls::Fraction(-3, 2), third_style);

    scls::Video_Encoder video = scls::Video_Encoder(path, duration / 60.0, width, height);
    PREPARE_TICK_SYSTEM;
    while(video.frame_count() > video.current_frame()){
        // Tick
        START_TICK(0);
        int tracing_time = 20;
        TICK_CURRENT_EVENT(20, f_1->set_link_drawing_proportion(0, event_proportion);, f_1->set_link_drawing_proportion(0, 1);)
        TICK_CURRENT_EVENT(20, f_2->set_link_drawing_proportion(0, event_proportion);, f_2->set_link_drawing_proportion(0, 1);f_1->set_link_drawing_proportion(0, 1);)
        TICK_CURRENT_EVENT(20, f_3->set_link_drawing_proportion(0, -event_proportion);, f_1->set_link_drawing_proportion(2, 1);f_3->set_link_drawing_proportion(0, 1);)
        TICK_WAIT(60);

        // Drawing B and H
        TICK_CURRENT_EVENT(20, l_1->set_link_drawing_proportion(0, event_proportion);l_2->set_link_drawing_proportion(0, -event_proportion);, l_1->set_link_drawing_proportion(0, 1);l_2->set_link_drawing_proportion(0, 1);)
        TICK_CURRENT_EVENT(40, first_style.get()->set_color(scls::Color(0, 0, 0, 255.0 * event_proportion));, first_style.get()->set_color(scls::Color(0, 0, 0, 255));)
        TICK_WAIT(60);

        // Draw the two others triangles
        TICK_CURRENT_EVENT(20,
                           f_2->set_link_drawing_proportion(2, -event_proportion);
                           l_3->set_link_drawing_proportion(0, event_proportion);
                           second_style.get()->set_color(scls::Color(0, 0, 0, 255.0 * event_proportion));
                           ,
                           f_2->set_link_drawing_proportion(2, 1);
                           l_3->set_link_drawing_proportion(0, 1);
                           second_style.get()->set_color(scls::Color(0, 0, 0, 255));)
        TICK_CURRENT_EVENT(20, f_2->set_link_drawing_proportion(1, -event_proportion);, f_2->set_link_drawing_proportion(1, 1);f_2->set_color(scls::Color(0, 0, 255, 255));)
        TICK_CURRENT_EVENT(20, f_3->set_link_drawing_proportion(1, event_proportion);, f_3->set_link_drawing_proportion(1, 1);)
        TICK_CURRENT_EVENT(20, f_3->set_link_drawing_proportion(2, event_proportion);, f_3->set_link_drawing_proportion(2, 1);f_3->set_color(scls::Color(0, 125, 255, 255));)
        TICK_WAIT(60);

        TICK_CURRENT_EVENT(40,
                           ft_1->set_opacity(event_proportion);
                           ft_2->set_opacity(event_proportion);
                           ft_3->set_opacity(event_proportion);
                           ft_4->set_opacity(event_proportion);
                           ft_5->set_opacity(event_proportion);
                           l_1->set_opacity(1.0 - event_proportion);l_2->set_opacity(1.0 - event_proportion);l_3->set_opacity(1.0 - event_proportion);
                           first_style.get()->set_color(scls::Color(0, 0, 0, 255.0 - 255.0 * event_proportion));
                           second_style.get()->set_color(scls::Color(0, 0, 0, 255.0 - 255.0 * event_proportion));
                           third_style.get()->set_color(scls::Color(0, 0, 0, 255.0 * event_proportion));
                           ,
                           ft_1->set_opacity(1);
                           ft_2->set_opacity(1);
                           ft_3->set_opacity(1);
                           ft_4->set_opacity(1);
                           ft_5->set_opacity(1);
                           l_1->set_opacity(0);l_2->set_opacity(0);l_3->set_opacity(0);
                           first_style.get()->set_color(scls::Color(0, 0, 0, 0));
                           second_style.get()->set_color(scls::Color(0, 0, 0, 0));
                           third_style.get()->set_color(scls::Color(0, 0, 0, 255));third_style_big.get()->set_color(scls::Color(0, 0, 0, 255));)
        TICK_WAIT(60);

        TICK_CURRENT_EVENT(40,
                           ft_2->move_y(scls::Fraction(-1, 20));ft_2->set_opacity(1.0 - event_proportion);
                           ft_3->move_x(scls::Fraction(-1, 40));
                           t_5->move_from_to(t_5_pos, t_5_pos - pleos::Vector(1, 0), event_proportion);
                           t_4->move_from_to(t_4_pos, t_4_pos - pleos::Vector(0, 2), event_proportion);t_4->set_opacity(1.0 - event_proportion);
                           t_6->set_opacity(1.0 - event_proportion);t_7->set_opacity(event_proportion);
                           ,
                           ft_2->move_y(scls::Fraction(-1, 20));ft_2->set_opacity(0);
                           ft_3->move_x(scls::Fraction(-1, 40));
                           t_5->move_from_to(t_5_pos, t_5_pos - pleos::Vector(1, 0), 1);t_5_pos = t_5->position();
                           t_4->move_from_to(t_4_pos, t_4_pos - pleos::Vector(0, 2), 1);t_4->set_opacity(0);
                           t_6->set_opacity(0);t_7->set_opacity(1);)
        TICK_WAIT(60);

        TICK_CURRENT_EVENT(40,
                           t_7->set_opacity(1.0 - event_proportion);t_8->set_opacity(event_proportion);
                           ,
                           t_7->set_opacity(0);t_8->set_opacity(1);)
        TICK_WAIT(60);

        TICK_CURRENT_EVENT(40,
                           ft_1->move_x(scls::Fraction(1, 80));
                           ft_3->move_y(scls::Fraction(-1, 20));ft_3->set_opacity(1.0 - event_proportion);
                           t_5->move_from_to(t_5_pos, t_5_pos - pleos::Vector(0, 2), event_proportion);t_5->set_opacity(1.0 - event_proportion);
                           t_8->set_opacity(1.0 - event_proportion);t_9->set_opacity(event_proportion);
                           ,
                           ft_1->move_x(scls::Fraction(1, 80));
                           ft_3->move_y(scls::Fraction(-1, 20));ft_3->set_opacity(0);
                           t_5->move_from_to(t_5_pos, t_5_pos - pleos::Vector(0, 2), 1);t_5->set_opacity(0);
                           t_8->set_opacity(0);t_9->set_opacity(1);)

        // Draw the image
        std::shared_ptr<scls::Image> current_image = graphic.get()->to_image(width, height);
        video.write_video_frame(current_image);
        video.go_to_next_frame();
        END_TICK;
    }

    video.close_encoding();
}

//******************
//
// Audio test
//
//******************

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
std::string frame_to_time(double current_frame){int current_minut = 0;while(current_frame > 60.0){current_frame -= 60.0;current_minut++;}return scls::format_number_to_text_strict(current_minut, 2, 0) + std::string(":") + scls::format_number_to_text_strict(current_frame, 2, 3);}
void audio_test(std::string path) {
    // Create the output video
    int duration = 60;int width = 1080;int height = 1920;
    double current_frame = 0;double last_frame = 0;double last_frame_silent = 0;
    std::string total_chain_content = std::string("");
    std::string current_chain = std::string();std::string total_chain = std::string();int current_chain_index = 0;
    std::string mlt_first_part = scls::replace(scls::replace(scls::read_file("assets/shotcut_datas/mlt_1_file.txt"), "WIDTH", std::to_string(width)), "HEIGHT", std::to_string(height));
    std::string mlt_second_part = scls::read_file("assets/shotcut_datas/mlt_2_file.txt");
    std::string mlt_third_part = scls::read_file("assets/shotcut_datas/mlt_3_file.txt");
    std::string mlt_path = std::string("E:/Youtube/Fichier shotcut/espace affine/espace affine.mlt");

    // Create the input video
    std::shared_ptr<scls::Image> audio_description = std::make_shared<scls::Image>(10000, 200, scls::Color(255, 255, 255));
    std::string input_path = std::string("J:/Youtube/Capture/2025-05-13 21-10-42.mp4");
    scls::Video_Decoder video_decoder = scls::Video_Decoder(input_path);

    // Decode the target
    int x = 0;
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
        double minimum_chain_time = 0.2;double minimum_silence_time = 0.134;double volume_action = 0.5;
        if(current_chain == std::string()) {
            if(std::abs(sample_max) > volume_action) {
                current_chain = std::string("    <entry producer=\"chain") + std::to_string(current_chain_index) + std::string("\" in=\"00:00:") + frame_to_time(current_frame - 0.067) + std::string("\"");
                last_frame = current_frame;last_frame_silent=0;
            }
        }
        else {
            if(std::abs(sample_max) < volume_action) {
                if(last_frame_silent > minimum_silence_time) {
                    if(current_frame - last_frame > minimum_chain_time) {
                        current_chain += std::string(" out=\"00:") + frame_to_time(current_frame) + std::string("\"/>");
                        total_chain += current_chain + std::string("\n");
                        total_chain_content += std::string("  <chain id=\"chain") + std::to_string(current_chain_index) + std::string("\"><property name=\"resource\">") + input_path + std::string("</property></chain>\n");
                        current_chain_index++;
                    }

                    current_chain = std::string();
                }

                last_frame_silent += AUDIO_TIME;
            }
            else{last_frame_silent=0;}
        }

        // Draw the part
        int bar_width = 2;int bar_height = std::abs(sample_average) * 200.0;
        int bar_max_height = std::abs(sample_max) * 200.0;
        if(current_chain != std::string()){audio_description.get()->fill_rect(x, 0, bar_width, audio_description.get()->height(), scls::Color(0, 255, 0));}
        audio_description.get()->fill_rect(x, audio_description.get()->height() - bar_max_height, bar_width, bar_max_height, scls::Color(130, 130, 130));
        audio_description.get()->fill_rect(x, audio_description.get()->height() - bar_height, bar_width, bar_height, scls::Color(0, 0, 0));x += bar_width;

        current_frame += AUDIO_TIME;
    }

    scls::write_in_file("chain.txt", total_chain);
    scls::write_in_file("chain_long.txt", total_chain_content);
    scls::write_in_file(mlt_path, mlt_first_part + total_chain_content + mlt_second_part + total_chain + mlt_third_part);
    audio_description.get()->save_png("test.png");
}

//******************
//
// New video
//
//******************

void new_video(std::string path, std::string thumbnail_path) {
    // Datas for the video
    double duration = 600.0;
    int height = 1920;int width = 1080;
    // Create the video generator
    Video_Generator video_generator = Video_Generator(path, duration, width, height);

    // Needed image
    std::shared_ptr<scls::Image> aster_system_logo_shared_ptr = scls::aster_system_logo(300);scls::Image* aster_system_logo = aster_system_logo_shared_ptr.get();
    std::shared_ptr<scls::Image> needed_thumbnail = std::make_shared<scls::Image>(thumbnail_path);
    std::shared_ptr<scls::Image> youtube_logo_shared_ptr = std::make_shared<scls::Image>(std::string("E:/Youtube (ancien)/fichier ajout video/fichier image/youtube.png")).get()->resize_adaptative(443, 300);
    scls::Image* youtube_logo = youtube_logo_shared_ptr.get();
    // Needed text
    scls::Text_Image_Generator tig;scls::Text_Style style;style.set_alignment_horizontal(scls::Alignment_Horizontal::H_Center);style.set_background_color(scls::Color(0, 0, 0, 0));style.set_font_size(100);
    style.set_color(scls::Color(255, 255, 255));
    std::shared_ptr<scls::Image> text_1_shared_ptr = tig.image_shared_ptr("Nouvelle vidéo", style);scls::Image* text_1 = text_1_shared_ptr.get();
    style.set_color(scls::Color(0, 0, 0));style.set_font_size(40);
    std::shared_ptr<scls::Image> text_2_shared_ptr = tig.image_shared_ptr("Le Programme De Mathématiques En Terminale !</br>Présentation Programme De Mathématiques Trimestre 2", style);scls::Image* text_2 = text_2_shared_ptr.get();
    style.set_color(scls::Color(255, 255, 255));style.set_font_size(100);
    std::shared_ptr<scls::Image> text_3_shared_ptr = tig.image_shared_ptr("Expérimental", style);scls::Image* text_3 = text_3_shared_ptr.get();

    // Loop
    while(video_generator.frame_count() > video_generator.current_frame()){
        // Start tick
        video_generator.start_frame(0);

        // Draw the image
        video_generator.set_current_image(std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255)));

        // Aster system logo
        if(video_generator.tick_between(40, 60)) {
            video_generator.current_image()->paste_bottom_center(aster_system_logo, video_generator.ticked_value(40, 20, 200 - aster_system_logo->height(), 200).to_double());
        }
        else if(video_generator.tick_above(60)) {video_generator.current_image()->paste_bottom_center(aster_system_logo, 200);}

        // Youtube logo
        if(video_generator.tick_between(40, 60)) {
            video_generator.current_image()->paste_top_center(youtube_logo, video_generator.ticked_value(40, 20, 200 - youtube_logo->height(), 220).to_double());
        }
        else if(video_generator.tick_above(60)) {video_generator.current_image()->paste_top_center(youtube_logo, 220);}

        // Handle drawing
        video_generator.fill_rect_from_left(0, 40, 0, 0, 1080, 200, scls::Color(255, 0, 0));
        video_generator.fill_rect_from_right(0, 40, 0, 1720, 1080, 200, scls::Color(255, 0, 0));

        // Text
        if(video_generator.tick_above(60)) {
            video_generator.current_image()->paste_top_center(text_1, 100 - text_1->height() / 2);
            video_generator.current_image()->paste_top_center(text_2, 520);
            video_generator.current_image()->paste_bottom_center(text_3, 100 - text_3->height() / 2);
        }

        // Handle thumbnail
        if(video_generator.tick_between(0, 40)) {
            video_generator.current_image()->paste_center(needed_thumbnail.get()->resize_adaptative(video_generator.ticked_point_2d(0, 40, 720, 405, 160, 90)));
        }
        else if(video_generator.tick_between(40, 60)) {
            video_generator.current_image()->paste_center(needed_thumbnail.get()->resize_adaptative(video_generator.ticked_point_2d(40, 20, 160, 90, 960, 540)));
        }
        else {video_generator.current_image()->paste_center(needed_thumbnail.get()->resize_adaptative(video_generator.ticked_point_2d(50, 600, 960, 540, 560, 315)));}

        // Finish the frame
        video_generator.go_to_next_frame();
    }

    video_generator.close_encoding();
}

//******************
//
// Main
//
//******************

long long current = 0;
int main(int argc, char **argv) {
    current = scls::time_ns();
    audio_test("test.mp4");

    std::cout << "Temps : " << static_cast<double>(scls::time_ns() - current) / 1000000000.0 << std::endl;

    return 0;
}
