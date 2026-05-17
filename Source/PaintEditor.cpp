#include "PaintEditor.h"
#include "Utilities.h"
#include "Color_Picker.h"
#include "Editor.h"
#include <raymath.h>

PaintEditor::PaintEditor(Project& project_ref, CameraController& camera_ref) :
	color_picker(),
	paint_menu(),
	project(project_ref),
	camera_controller(camera_ref)
{
	Build_paint_menu();

	camera_controller.Set_birds_eye();
	camera_controller.Set_projection(CAMERA_PERSPECTIVE);
	camera_controller.Set_target(color_picker.position);
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

std::vector<std::string> PaintEditor::Paint_stats(const Paint& paint)
{
	std::vector<std::string> stats;

	std::string m2_per_liter{ "M2 per Liter: " + std::to_string(paint.m2_per_liter) };
	m2_per_liter.erase(m2_per_liter.find_last_not_of('0') + 1, std::string::npos);
	m2_per_liter.erase(m2_per_liter.find_last_not_of('.') + 1, std::string::npos);

	stats.push_back(m2_per_liter);
	stats.push_back("Strykningar: " + std::to_string(paint.coats));
	return stats;

}


std::unique_ptr<State> PaintEditor::Update()
{
	if (IsKeyReleased(KEY_TAB))
	{
		return std::make_unique<Editor>(project, camera_controller);
	}
	color_picker.Update(camera_controller.camera);



	Paint* selected_paint;
	const int i = paint_menu.Selected_index();
	if (i < 0)
	{
		selected_paint = nullptr;
	}
	else
	{
		selected_paint = &project.paints.at(i);
	}

	const Rectangle selected_paint_rec{ 0.1f * GetScreenWidthF(), 0.1f * GetScreenHeightF(), 0.2f * GetScreenHeightF(), 0.2f * GetScreenHeightF() };
	const Rectangle selected_stats_rec{ selected_paint_rec.x, selected_paint_rec.y + selected_paint_rec.height, selected_paint_rec.width, selected_paint_rec.height };
	if (selected_paint)
	{
		Rect_list rec_list{ Paint_stats(*selected_paint), selected_stats_rec };

		selected_paint->Draw_swatch(selected_paint_rec); //move to render
		rec_list.Draw(LIGHTGRAY);

		if (CheckCollisionPointRec(GetMousePosition(), rec_list.Entry_rectangle(0)))
		{

			const float wheel = GetMouseWheelMove();
			rec_list.Draw_line(WHITE, 0);
			if (wheel != 0)
			{

				selected_paint->m2_per_liter += wheel * 0.5f;
				selected_paint->m2_per_liter = Clamp(selected_paint->m2_per_liter, 0.0f, 50.0f);

			}
		}
		else
		{
			camera_controller.Update_zoom();

		}
	}
	else
	{

		camera_controller.Update_zoom();

	}

	if (selected_paint)
	{

		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
		{
			const int selected = color_picker.Hovered_index();
			if (selected > -1)
			{
				const RGB color = color_picker.Get_color();
				selected_paint->color = color;
			}
		}

		//rec_list.Draw_outline();
	}
	paint_menu.Update({ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF() }); //TODO: repeated magic menu-position(make member paint_menu_position) //Todo: needs to be put after selectionlogic due to internal input-handling

	return nullptr;
}



void PaintEditor::Render() const
{
	camera_controller.Begin_3D();
	color_picker.Draw();
	camera_controller.End_3D();

	paint_menu.Draw({ 0.8f * GetScreenWidthF(), 0.2f * GetScreenHeightF() }); //TODO: repeated magic menu-position
	

}
