#pragma once
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raylib.h"
#pragma warning(pop)

#include <memory>
#include <vector>

struct Menu_Icon
{
    virtual ~Menu_Icon() = default;
    virtual void Draw(Rectangle rect, bool selected) const = 0;
};

class Menu
{
public:
    void Add_item(std::unique_ptr<Menu_Icon> item)
    {
        items.push_back(std::move(item));
    }
    
    void Update(Vector2 position) noexcept;
    void Draw(Vector2 position) const;
    int  Selected_index() const noexcept { return selected; }

private:
    std::vector<std::unique_ptr<Menu_Icon>> items;
    Rectangle Icon(Vector2 position, size_t index) const noexcept;
    int selected = -1;
};
