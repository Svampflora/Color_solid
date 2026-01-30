#include "Color_Picker.h"
#include "FloorPlanEditor.h"

#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable:ALL_CODE_ANALYSIS_WARNINGS)
//#include "raylib.h"
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
	solid( 2.0f, 3.0f, 12, 6, Color_wheel()),
	rotation(QuaternionIdentity()),
	target_rotation(QuaternionIdentity()),
	position{ 0.0f, 0.0f, 0.0f },
	angular_velocity{ 0.0f, 0.0f, 0.0f }
{
}

RGB Color_picker::Get_color()
{
	return solid.node(hovered).color;
}

void Color_picker::Update(const Camera& camera)
{
	Mouse_rotation(camera);
	Node_selection(camera);
	
	////++++++++++++++++++++++++++++++++
	//const Quaternion q_delta = QuaternionMultiply(
	//	target_rotation,
	//	QuaternionInvert(rotation)
	//);

	//Vector3 axis;
	//float angle;
	//QuaternionToAxisAngle(q_delta, &axis, &angle);



	//angular_velocity = Vector3Add(
	//	angular_velocity,
	//	Vector3Scale(axis, angle * STIFFNESS * GetFrameTime())
	//);

	//angular_velocity = Vector3Scale(
	//	angular_velocity,
	//	1.0f - DAMPING * GetFrameTime()
	//);

	//rotation = IntegrateRotation(rotation, angular_velocity, GetFrameTime());
}

void Color_picker::Draw()  const
{
	for (const auto& node : solid.color_nodes)
	{
		// rotate local ? world
		const Vector3 world_pos =
			Vector3Add(position,
				Vector3RotateByQuaternion(node.position, rotation));

		DrawSphere(world_pos, COLOR_NODE_RADIUS, node.color);
	}

	if (hovered > -1)
	{
		const Color_node node = solid.node(hovered);
		const Vector3 world_pos =
			Vector3Add(position,
				Vector3RotateByQuaternion(node.position, rotation));
		DrawSphere(world_pos, COLOR_NODE_RADIUS * 1.2f, node.color);

	}
}

void Color_picker::Mouse_rotation(const Camera& camera)
{
	if (!IsMouseButtonDown(MOUSE_RIGHT_BUTTON))
		return;

	const Vector2 delta = GetMouseDelta();
	if (Vector2Length(delta) < 0.001f)
		return;

	const float dist = Vector3Distance(camera.position, position);
	const float sens = STIFFNESS * dist * 0.2f;

	const Vector3 world_up = { 0, 1, 0 };   
	const Vector3 cam_right = Vector3Normalize(Vector3CrossProduct(camera.up, Vector3Subtract(camera.target, camera.position)));
	 
	const float yaw = delta.x * sens;
	const float pitch = -delta.y * sens;
	 
	const Quaternion q_yaw = QuaternionFromAxisAngle(world_up, yaw);
	const Quaternion q_pitch = QuaternionFromAxisAngle(cam_right, pitch);

	rotation = QuaternionMultiply(q_yaw, rotation);
	rotation = QuaternionMultiply(q_pitch, rotation);



}

void Color_picker::Node_selection(const Camera& camera)
{
	const Ray ray = GetMouseRay(GetMousePosition(), camera);

	float closest_distance = Vector3Distance(camera.position, position); //TODO: make default distance longer
	int i = 0;
	for (const auto& node : solid.color_nodes)
	{
		const Vector3 world_pos =
			Vector3Add(position,
				Vector3RotateByQuaternion(node.position, rotation));

		const RayCollision collision = GetRayCollisionSphere(ray, world_pos, COLOR_NODE_RADIUS);

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

	if (closest_distance == Vector3Distance(camera.position, position))
	{
		hovered = -1;
	}
}
