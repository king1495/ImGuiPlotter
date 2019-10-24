#pragma once
#include <cmath>
#include <vector>
#include <string>

#include <ImGui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <ImGui/imgui_internal.h>

struct IMGUI_API ImCircle {
	ImVec2 center;
	float radius;
	ImCircle() : center(0, 0), radius(FLT_MAX) {};
	ImCircle(float r, ImVec2 c = ImVec2()) : center(c), radius(r) {};
};

inline bool ImPointinRect(const ImRect& rc, const ImVec2& p) {
	return p.x >= rc.Min.x && p.y >= rc.Min.y && p.x <= rc.Max.x && p.y <= rc.Max.y;
}

inline bool ImPointinCircle(const ImCircle& rc, const ImVec2& p) {
	return sqrt(ImLengthSqr(p - rc.center)) <= rc.radius;
}

// class generator:
struct c_unique {
	int current_;
	c_unique(int start) : current_(start) {}
	int operator() () { return current_++; }
};

enum ImPlotCoordType_ {
	ImPlotCoordType_Cartesian,
	ImPlotCoordType_Polar,
};

enum ImPlotLineStyle_ {
	ImPlotLineStyle_None,
	ImPlotLineStyle_Line,
	//ImPlotLineStyle_Dotted
};

enum ImPlotMarkerStyle_ {
	ImPlotMarkerStyle_None,
	ImPlotMarkerStyle_Circle,
	ImPlotMarkerStyle_Rect
};

template<typename T>
class ImPlot
{
public:
	ImPlot()
		: axesType(ImPlotCoordType_Cartesian)
		, lineStyle(ImPlotLineStyle_Line), lineColor(ImGui::GetColorU32(ImGuiCol_PlotLines)), lineWidth(1.f)
		, markerStyle(ImPlotMarkerStyle_None), markerColor(ImGui::GetColorU32(ImGuiCol_PlotLines)), markerSize(2.f)
		, data_bb(ImRect()), canvas_bb(ImRect())
	{};
	virtual ~ImPlot() {};

	void SetData(const std::vector<T>& _y) {
		this->vXdata.resize(_y.size());
		c_unique UniqueNumber(0);
		generate(vXdata.begin(), vXdata.end(), UniqueNumber);
		this->vYdata = _y;
	}

	void SetData(const std::vector<T>& _x, const std::vector<T>& _y) {
		this->vYdata = _y;
		this->vXdata = _x;
	}

	ImRect GetDataMinMax() { return data_bb; }

	void Render();

	template<typename T>
	friend class ImGuiPlotter;

	template<typename T>
	friend class ImAxes;

private:
	std::vector<T> vXdata; // Radius
	std::vector<T> vYdata; // Theta

public:
	ImPlotLineStyle_ lineStyle;
	ImColor lineColor;
	float lineWidth;

	ImPlotMarkerStyle_ markerStyle;
	ImColor markerColor;
	float markerSize;
private:
	ImPlotCoordType_ axesType;
	ImRect canvas_bb;
	ImRect data_bb;
};

template<typename T>
inline void ImPlot<T>::Render()
{
	using namespace ImGui;

	if ((vYdata.size() != vXdata.size()) | vYdata.empty() | vXdata.empty()) return;

	ImGuiWindow* window = GetCurrentWindow();

	ImVec2 steps = ImVec2(1.f, 1.f) / data_bb.GetSize();
	ImCircle data_cc(data_bb.Max.y, data_bb.GetCenter());

	T y0, x0;
	T y1, x1;
	bool dataRender = false;

	if (axesType == ImPlotCoordType_Polar) {
		T radius = vYdata[0];
		T theta = vXdata[0];
		x0 = radius * cos(theta);
		y0 = radius * sin(theta);
		dataRender = ImPointinCircle(data_cc, ImVec2(x0, y0));
		//dataRender = radius <= data_bb.Max.y & radius >= data_bb.Min.x;
	}
	else {
		y0 = vYdata[0];
		x0 = vXdata[0];
		dataRender = ImPointinRect(data_bb, ImVec2(x0, y0));
	}

	ImVec2 xy0 = ImVec2((x0 - data_bb.Min.x) * steps.x, 1.0f - ((y0 - data_bb.Min.y)) * steps.y);
	ImVec2 pos0 = ImLerp(canvas_bb.Min, canvas_bb.Max, xy0);
	if (dataRender) {
		switch (markerStyle)
		{
		case ImPlotMarkerStyle_Circle:
			window->DrawList->AddCircleFilled(
				pos0, markerSize
				//, colorData[0]);
				, markerColor);
			break;
		case ImPlotMarkerStyle_Rect:
			window->DrawList->AddRectFilled(
				pos0 - ImVec2(0.5f * markerSize, 0.5f * markerSize)
				, pos0 + ImVec2(0.5f * markerSize, 0.5f * markerSize)
				//, colorData[0]);
				, markerColor);
			break;
		default:
			break;
		}
	}

	for (int n = 1; n < vYdata.size(); n++)
	{
		if (axesType == ImPlotCoordType_Polar) {
			T radius = vYdata[n];
			T theta = vXdata[n];
			x1 = radius * cos(theta);
			y1 = radius * sin(theta);
			dataRender = ImPointinCircle(data_cc, ImVec2(x1, y1));
			//dataRender = radius <= data_bb.Max.y & radius >= data_bb.Min.x;
		}
		else {
			y1 = vYdata[n];
			x1 = vXdata[n];
			dataRender = ImPointinRect(data_bb, ImVec2(x1, y1));
		}

		ImVec2 xy1 = ImVec2((x1 - data_bb.Min.x) * steps.x, 1.0f - ((y1 - data_bb.Min.y)) * steps.y);
		ImVec2 pos1 = ImLerp(canvas_bb.Min, canvas_bb.Max, xy1);
		if (dataRender) {
			switch (lineStyle)
			{
			case ImPlotLineStyle_Line:
				window->DrawList->AddLine(
					pos0
					, pos1
					//, colorData[n]
					, lineColor
					, lineWidth);
				break;
			default:
				break;
			}
			switch (markerStyle)
			{
			case ImPlotMarkerStyle_Circle:
				window->DrawList->AddCircleFilled(
					pos1, markerSize
					//, colorData[n]);
					, markerColor);
				break;
			case ImPlotMarkerStyle_Rect:
				window->DrawList->AddRectFilled(
					pos1 - ImVec2(0.5f * markerSize, 0.5f * markerSize)
					, pos1 + ImVec2(0.5f * markerSize, 0.5f * markerSize)
					//, colorData[n]);
					, markerColor);
				break;
			default:
				break;
			}
		}
		pos0 = pos1;
	}
}
