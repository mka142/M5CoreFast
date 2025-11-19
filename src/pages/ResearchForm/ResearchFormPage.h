#pragma once
#include <FormPageBase.h>

class ResearchFormPage : public FormPageBase {
public:
    static lv_obj_t* create();
    static void cleanup();
    
protected:
    // Implement abstract methods from FormPageBase
    void create_intro_screen() override;
    Question* get_question(int page) override;
    const char* get_section_header(int page) override;
    void on_form_submit() override;
    const char* get_intro_title() override;
    const char* get_intro_text() override;
    
private:
    static ResearchFormPage* instance;
};
