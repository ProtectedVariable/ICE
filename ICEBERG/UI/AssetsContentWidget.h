#pragma once

#include <imgui.h>

#include <memory>
#include <string>
#include <vector>

#include "Widget.h"

struct AssetView {
    std::string folder_name;
    std::vector<std::pair<std::string, void *>> assets;
    std::vector<std::shared_ptr<AssetView>> subfolders;
};

class AssetsContentWidget : public Widget {
   public:
    AssetsContentWidget() = default;

    void render() override {
        
    }

   private:
    
};
