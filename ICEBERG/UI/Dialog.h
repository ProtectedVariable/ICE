#pragma once

#include "Widget.h"

enum class DialogResult { None, Pending, Ok, Cancel };

class Dialog : public Widget {
   public:
    Dialog() { m_dialog_id = DIALOG_ID++; }
    virtual ~Dialog() = default;

    void open() {
        m_open_request = true;
        m_result = DialogResult::Pending;
        m_is_open = true;
    }

    void done(DialogResult result) {
        m_is_open = false;
        m_result = result;
    }

    bool isOpenRequested() {
        if (m_open_request) {
            m_open_request = false;
            return true;
        }
        return false;
    }

    bool isOpen() const { return m_is_open; }

    DialogResult getResult() const { return m_result; }

   protected:
    int m_dialog_id;

   private:
    static int DIALOG_ID;
    bool m_open_request = false;
    bool m_is_open = false;
    DialogResult m_result = DialogResult::None;
};