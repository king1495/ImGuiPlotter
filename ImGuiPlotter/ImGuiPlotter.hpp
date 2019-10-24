#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <memory>

#include <ImGui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>

#include "ImAxes.hpp"
#include "ImPlot.hpp"

template<typename T>
class ImGuiPlotter
{
public:
	ImGuiPlotter()
		:frameSize(640, 480)
		, faceColor(ImGui::GetColorU32(ImGuiCol_FrameBg))
		, subPlot(1, 1)
	{
		vAxes.resize(1, nullptr);
	};
	~ImGuiPlotter() {};

	void SubPlot(int _row, int _col) {
		if (_row < 1 | _col < 1) return;
		subPlot = { (float)_col, (float)_row };
		ClearImAxes();
	}

	void AddImAxes(std::shared_ptr<ImAxes<T>> _axes, int _ind = 0) {
		if (_ind < 0 | _ind >= vAxes.size()) return;
		vAxes[_ind] = _axes;
	}

	void ClearImAxes() {
		vAxes.resize(subPlot.x * subPlot.y, nullptr);
	}

	void Render();

	template<typename T>
	friend class ImAxes;
	template<typename T>
	friend class ImPlot;

private:
	std::vector<std::shared_ptr<ImAxes<T>>> vAxes;
	ImVec2 subPlot;

public:
	ImColor faceColor;
	ImVec2 frameSize;

private:
	ImRect frame_bb;
	ImRect inner_bb;
};

template<typename T>
inline void ImGuiPlotter<T>::Render()
{
	using namespace ImGui;
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return;
	ImGuiStyle style = GetStyle();

	// Frame Setup and Draw
	frame_bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + frameSize);
	inner_bb = ImRect(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);

	ItemSize(frame_bb, 0);
	if (!ItemAdd(frame_bb, 0, &frame_bb))
		return;

	RenderFrame(frame_bb.Min, frame_bb.Max, faceColor, true, style.FrameRounding);

	ImVec2 AxesSize = (inner_bb.GetSize() - (style.FramePadding * (subPlot - ImVec2(1.f, 1.f)))) / subPlot;

	int subNum = subPlot.x * subPlot.y;
	for (int i = 0; i < (vAxes.size() > subNum ? subNum : vAxes.size()); ++i) {
		if (vAxes[i] == nullptr) continue;
		ImVec2 xy0 = inner_bb.Min + (AxesSize + style.FramePadding) * ImVec2(i % (int)(subPlot.x), std::floor(i / subPlot.x));
		vAxes[i]->axes_bb = ImRect(xy0, xy0 + AxesSize);
		vAxes[i]->Render();
	}
}
