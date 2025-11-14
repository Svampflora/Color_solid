#include "Color_Picker.h"

#include "FloorPlanEditor.h"




template<typename T, typename Func>
void CheckConditionAndPerformAction(T value, Func action)
{
	if (value)
	{
		action();
	}
}

template <typename T>
void RenderObjects(const std::vector<T>& objects, const Texture& texture) noexcept
{
	for (const T& obj : objects)
	{
		obj.Render(texture);
	}
}

template <typename T>
void UpdateObjects(std::vector<T>& objects)  noexcept
{
	for (T& obj : objects)
	{
		obj.Update();
	}
}

template <typename Container, typename Predicate>
void remove_if(Container& container, Predicate predicate)
{
	container.erase(
		std::remove_if(container.begin(), container.end(), predicate),
		container.end());
}



std::unique_ptr<State> Color_Picker::Update()
{
	//if (IsKeyReleased(KEY_Q))
	//{
	//	EndMode3D();
	//	return std::make_unique<FloorPlanEditor>();
	//}
	//
	//const float scroll = GetMouseWheelMove();
	//solid.rotation.y += scroll * 5.0f;

	return nullptr;
}

[[gsl::suppress(f.6)]]
Color_Picker::Color_Picker() :
	wheel({
	{0.0f, NCS_YELLOW},
	{PI / 2, NCS_RED},
	{PI , NCS_BLUE},
	{3 * PI / 2, NCS_GREEN}
		}),
	solid({ 0.0f, 0.0f, -4.0f }, 2.0f, 3.0f, 48, 12, wheel)

{
	Camera camera = { 0 };
	//camera.position = Vector3{ 0.0f, 10.0f, 10.0f };
	camera.position = { 0.0f, 2.0f, 6.0f };

	camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;

}

void Color_Picker::Render()  const
{
	//Color targetRGB = NCS_To_RGB(ncsInput);
	//DrawRectangle(400, 100, 100, 100, targetRGB);
	//DrawText(ncsInput.c_str(), 400, 210, 20, WHITE);
	NCS_Color ncs_color(ncsInput);
	//ncs_color.draw({ 400, 100 }, { 100, 100 });

	//NCSTriangle ncs_triangle(ncs_color.hueCode, 10);

	//ncs_triangle.draw({ 0.1f * GetScreenWidthF(), 0.1f * GetScreenHeightF() }, { 0.8f * GetScreenWidthF(), 0.8f * GetScreenHeightF() });




	//wheel.draw({ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f }, 250, 12);
	//DrawCircleV({ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f }, 250, GRAY);

	//drawTriangleGradient({ GetScreenWidthF() * 0.5f, 0.0f }, { 0.0f, GetScreenHeightF() }, { GetScreenWidthF(), GetScreenHeightF() }, NCS_RED, NCS_YELLOW, 6);

	solid.draw();

}
