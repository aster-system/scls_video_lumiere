#include "../headers/video_sort.h"

//******************
//
// Sort
//
//******************

void bubble_video(std::string path, int datas_number, int sort_width, int width, int height) {
    // Encode the video
    double speed = 1;
    std::shared_ptr<pleos::Sort_Datas> sort_datas = pleos::algorithms_sort_creation(datas_number);
    scls::Video_Encoder video = scls::Video_Encoder(path, (static_cast<double>(datas_number) / speed + 1.0) / 60.0, width, height);
    while(video.frame_count() > video.current_frame()){
        std::shared_ptr<scls::Image> current_image = std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255));
        current_image.get()->paste(pleos::algorithms_sort_image(current_image, sort_datas.get(), width / 2 - sort_width / 2, 100, sort_width, sort_width).get(), 0, 0);
        video.write_video_frame(current_image);
        video.go_to_next_frame();

        // Do the sort
        for(int i = 0;i<speed;i++){pleos::algorithms_comparaison_bubble(sort_datas.get());}
    }

    video.close_encoding();
}

void fusion_video(std::string path, int datas_number, int sort_width, int width, int height) {
    // Encode the video
    double speed = 4;
    std::shared_ptr<pleos::Sort_Datas> sort_datas = pleos::algorithms_sort_creation(datas_number);
    scls::Video_Encoder video = scls::Video_Encoder(path, (static_cast<double>(algorithms_comparaison_fusion_time(sort_datas.get())) / speed + 1.0) / 60.0, width, height);
    while(video.frame_count() > video.current_frame()){
        std::shared_ptr<scls::Image> current_image = std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255));
        current_image.get()->paste(pleos::algorithms_sort_image(current_image, sort_datas.get(), width / 2 - sort_width / 2, 100, sort_width, sort_width).get(), 0, 0);
        video.write_video_frame(current_image);
        video.go_to_next_frame();

        // Do the sort
        for(int i = 0;i<speed;i++){pleos::algorithms_comparaison_fusion(sort_datas.get());}
    }

    video.close_encoding();
}

void insertion_video(std::string path, int datas_number, int sort_width, int width, int height) {
    // Encode the video
    double speed = 2;
    std::shared_ptr<pleos::Sort_Datas> sort_datas = pleos::algorithms_sort_creation(datas_number);
    scls::Video_Encoder video = scls::Video_Encoder(path, (static_cast<double>(datas_number) / speed + 1.0) / 60.0, width, height);
    while(video.frame_count() > video.current_frame()){
        std::shared_ptr<scls::Image> current_image = std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255));
        current_image.get()->paste(pleos::algorithms_sort_image(current_image, sort_datas.get(), width / 2 - sort_width / 2, 100, sort_width, sort_width).get(), 0, 0);
        video.write_video_frame(current_image);
        video.go_to_next_frame();

        // Do the sort
        for(int i = 0;i<speed;i++){pleos::algorithms_comparaison_insertion(sort_datas.get());}
    }

    video.close_encoding();
}

void selection_video(std::string path, int datas_number, int sort_width, int width, int height) {
    // Encode the video
    double speed = 2;
    std::shared_ptr<pleos::Sort_Datas> sort_datas = pleos::algorithms_sort_creation(datas_number);
    scls::Video_Encoder video = scls::Video_Encoder(path, (static_cast<double>(datas_number) / speed + 1.0) / 60.0, width, height);
    while(video.frame_count() > video.current_frame()){
        std::shared_ptr<scls::Image> current_image = std::make_shared<scls::Image>(width, height, scls::Color(255, 255, 255));
        int sort_width = 600;
        current_image.get()->paste(pleos::algorithms_sort_image(current_image, sort_datas.get(), width / 2 - sort_width / 2, 100, sort_width, sort_width).get(), 0, 0);
        video.write_video_frame(current_image);
        video.go_to_next_frame();

        // Do the sort
        for(int i = 0;i<speed;i++){pleos::algorithms_comparaison_selection(sort_datas.get());}
    }

    video.close_encoding();
}

void sort_all(){
    // Hardcore mode
    bubble_video("test_bubble.mp4", 300, 600, 720, 1080);
    fusion_video("test_fusion.mp4", 900, 900, 1080, 1920);
    insertion_video("test_insertion.mp4", 900, 900, 1080, 1920);
    selection_video("test_selection.mp4", 900, 900, 1080, 1920);
};
