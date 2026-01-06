#include "Color_Picker.h"
#include "FloorPlanEditor.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
#include "raymath.h"
#pragma warning(pop)



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

[[gsl::suppress(f.6)]]
Color_picker::Color_picker() :
	solid({ 0.0f, 0.0f, 0.0f }, 2.0f, 3.0f, 12, 6, Color_wheel())

{}

void Color_picker::Update(const Camera& camera)
{

	const Ray ray = GetMouseRay(GetMousePosition(), camera);
	
	float closest_distance = Vector3Distance(camera.position, solid.center); //TODO: make default distance longer
	int i = 0;
	for (const auto& node : solid.color_nodes)
	{
		const RayCollision collision = GetRayCollisionSphere(ray, node.position, COLOR_NODE_RADIUS);


		if (collision.hit)
		{
			const float distance_to_hit = Vector3Distance(camera.position, collision.point);
			if (distance_to_hit < closest_distance)
			{
				closest_distance = distance_to_hit;
				hovered = i;
			}
		}

		i++;
	}

	if (closest_distance == Vector3Distance(camera.position, solid.center))
	{
		hovered = -1;
	}

	//if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
	//{
	//}
}



void Color_picker::Draw()  const
{
	solid.Draw();

	if (hovered > -1)
	{
		const Color_node node = solid.node(hovered);
		DrawSphere(node.position, COLOR_NODE_RADIUS * 1.2f, node.color); //solid.draw_node(index, size)?

	}
}
