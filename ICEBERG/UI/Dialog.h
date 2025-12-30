#pragma once

#include "Widget.h"

enum class DialogResult { None, Ok, Cancel };

class Dialog : public Widget {
   public:
    virtual ~Dialog() = default;

    void open() {
        m_open_request = true;
        m_result = DialogResult::None;
    }

    void done(DialogResult result) { m_result = result; }

    bool isOpenRequested() {
        if (m_open_request) {
            m_open_request = false;
            return true;
        }
        return false;
    }

    DialogResult getResult() const { return m_result; }

   private:
    bool m_open_request = false;
    DialogResult m_result = DialogResult::None;
};