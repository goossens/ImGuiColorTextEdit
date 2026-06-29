//	TextEditor - A syntax highlighting text editor for ImGui
//	Copyright (c) 2024-2026 Johan A. Goossens. All rights reserved.
//
//	This work is licensed under the terms of the MIT license.
//	For a copy, see <https://opensource.org/licenses/MIT>.


#pragma once


//
//	Include files
//

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>
#include <vector>

#include "imgui.h"
#include "imgui_internal.h"


//
//	Notifications
//
//	This class provides a "toast" notification system. Customers only have to Add
//	notification by providing a type, message and a dismissal time in microseconds
//	(the default is 400ms or 4 seconds).
//
//	Notifications show up as a stack growing from the bottom to the top. Older
//	messages will fade out and collapse. The user simply calls the Render method
//	every frame with a bottom left coordinate in viewport space and this will update
//	the lifecycle of each notification an render the stack.

class Notifications {
public:
	// notifications types
	enum class Type {
		success,
		warning,
		error,
		info
	};

	// constructor
	Notifications() {}

	// add a new notification
	inline void Add(Type type, const std::string& message, int dismissTime=4000) {
		notifications.emplace_back(type, message, dismissTime);
	}

	// render notifications as a stack (position is bottom-right of stack in viewport space)
	inline void Render(ImVec2 pos) {
		// update state of all notifications
		for (auto& notification : notifications) {
			notification.update();
		}

		// remove expired notifications
		notifications.erase(std::remove_if(notifications.begin(), notifications.end(), [](const Notification& candidate) {
			return candidate.phase == Notification::Phase::expired;
		}), notifications.end());

		// render remaining notifications
		float offset = 0.0f;

		for (auto& notification : notifications) {
			offset = notification.render(pos, offset);
		}
	}

private:
	// a single notification
	struct Notification {
		// constructor
		Notification(Type type, const std::string& message, int dismissTime) : message(message) {
			std::stringstream ss;
			ss << "Notification" << id++;
			name = ss.str();

			switch (type) {
				case Type::success:
					title = "Success";
					titleColor = {0.0f, 255.0f, 0.0f, 255.0f};
					break;

				case Type::warning:
					title = "Warning";
					titleColor = {255.0f, 255.0f, 0.0f, 255.0f};
					break;

				case Type::error:
					title = "Error";
					titleColor = {255.0f, 0.0f, 0.0f, 255.0f};
					break;

				case Type::info:
					title = "Info";
					titleColor = {0.0f, 157.0f, 255.0f, 255.0f};
					break;
			}

			fadeInStart = std::chrono::system_clock::now();
			waitStart = fadeInStart + fadeInDuration;
			fadeOutStart = waitStart + std::chrono::milliseconds(dismissTime);
			ghostStart = fadeOutStart + fadeOutDuration;
			expiredStart = ghostStart + ghostDuration;
		}

		// update the notification state
		inline void update() {
			// get current time
			auto now = std::chrono::system_clock::now();

			// update phase
			if (now >= expiredStart) {
				phase = Phase::expired;

			} else if (now >= ghostStart) {
				phase = Phase::ghost;

			} else if (now >= fadeOutStart) {
				phase = Phase::fadeOut;

			} else if (now >= waitStart) {
				phase = Phase::wait;

			} else {
				phase = Phase::fadeIn;
			}

			// determine transparency
			using dmilliseconds = std::chrono::duration<float, std::milli>;

			if (phase == Phase::fadeIn) {
				alpha = std::chrono::duration_cast<dmilliseconds>(now - fadeInStart) / fadeInDuration;

			} else if (phase == Phase::wait) {
				alpha = 1.0f;

			} else if (phase == Phase::fadeOut) {
				alpha = 1.0f - (std::chrono::duration_cast<dmilliseconds>(now - fadeOutStart) / fadeOutDuration);

			} else {
				alpha = 0.0f;
			}

			titleColor.w = alpha;

			// update ghost height (if required)
			if (phase == Phase::ghost) {
				ghostHeight = (1.0f - (std::chrono::duration_cast<dmilliseconds>(now - ghostStart) / ghostDuration)) * height;
			}
		}

		// render a notification
		inline float render(ImVec2 pos, float offset) {
			if (phase == Notification::Phase::ghost) {
				offset += ghostHeight + ImGui::GetStyle().ItemSpacing.y;

			} else {
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
				ImGui::SetNextWindowBgAlpha(alpha);
				ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y - offset), ImGuiCond_Always, ImVec2(1.0f, 1.0f));

				ImGuiWindowFlags flags =
					ImGuiWindowFlags_NoSavedSettings |
					ImGuiWindowFlags_AlwaysAutoResize |
					ImGuiWindowFlags_NoDecoration |
					ImGuiWindowFlags_NoNav |
					ImGuiWindowFlags_NoBringToFrontOnFocus |
					ImGuiWindowFlags_NoFocusOnAppearing;

				ImGui::Begin(name.c_str(), nullptr, flags);
				ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

				ImGui::PushStyleColor(ImGuiCol_Text, titleColor);
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s", title.c_str());
				ImGui::PopStyleColor();

				auto buttonWidth = ImGui::CalcTextSize("x").x + ImGui::GetStyle().FramePadding.x * 2.0f;
				ImGui::SameLine(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonWidth);

				if (ImGui::Button("x")) {
					expiredStart = std::chrono::system_clock::now();
				}

				ImGui::PushTextWrapPos(ImGui::GetMainViewport()->Size.x / 4.0f);
				ImGui::AlignTextToFramePadding();
				ImGui::Text("%s", message.c_str());
				ImGui::PopTextWrapPos();

				height = ImGui::GetWindowHeight();
				offset += height + ImGui::GetStyle().ItemSpacing.y;

				ImGui::End();
				ImGui::PopStyleVar();
			}

			return offset;
		}

		// properties
		static constexpr std::chrono::milliseconds fadeInDuration{400};
		static constexpr std::chrono::milliseconds fadeOutDuration{400};
		static constexpr std::chrono::milliseconds ghostDuration{300};

		enum class Phase {
			fadeIn,
			wait,
			fadeOut,
			ghost,
			expired
		};

		inline static size_t id = 1;
		std::string name;
		Phase phase;
		std::string title;
		std::string message;
		ImVec4 titleColor;
		float alpha;
		float height;
		float ghostHeight;

		std::chrono::system_clock::time_point fadeInStart;
		std::chrono::system_clock::time_point waitStart;
		std::chrono::system_clock::time_point fadeOutStart;
		std::chrono::system_clock::time_point ghostStart;
		std::chrono::system_clock::time_point expiredStart;
	};

	// list of active notifications
	std::vector<Notification> notifications;
};
