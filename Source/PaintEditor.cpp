#include "PaintEditor.h"
#include "Utilities.h"
#include "Color_Picker.h"
#include "Editor.h"

PaintEditor::PaintEditor(Project& project_ref, CameraController& camera_ref) :
	color_picker(),
	paint_menu(),
	project(project_ref),
	camera_controller(camera_ref)
{
	Build_paint_menu();
}

void PaintEditor::Build_paint_menu()
{
	paint_menu = Menu{};

	for (const Paint& p : project.paints)
	{
		paint_menu.Add_item(
			std::make_unique<Paint_Icon>(&p, project.room)
		);
	}
}

std::unique_ptr<State> PaintEditor::Update()
{
	if (IsKeyReleased(KEY_TAB))
	{
		return std::make_unique<Editor>(project, camera_controller);
	}
	color_picker.Update(camera_controller.camera);
	paint_menu.Update({ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF() }); //TODO: repeated magic menu-position

	return nullptr;
}

void PaintEditor::Render() const
{
	camera_controller.Begin_3D();
	color_picker.Draw();
	camera_controller.End_3D();

	paint_menu.Draw({ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF() }); //TODO: repeated magic menu-position

}
