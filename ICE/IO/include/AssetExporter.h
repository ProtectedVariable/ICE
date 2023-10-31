#pragma once

template <typename T>
class AssetExporter {
    virtual void writeToJson() = 0;
};
