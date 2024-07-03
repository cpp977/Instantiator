#include "IO/ProgressBar.hpp"

#include <string>

#include <fmt/color.h>
#include <fmt/core.h>

void ProgressBar::step(std::string message)
{
    curr = curr + 1;
    draw(message);
}

void ProgressBar::update(int curr_in, std::string message)
{
    curr = curr_in;
    draw(message);
}

void ProgressBar::draw(std::string message) const
{
    int reps_done = static_cast<int>(curr * scaling);
    int reps_do = 100 - reps_done;
    std::string fmt_done = fmt::format("{{:{}>{}}}", fill_done, reps_done);
    std::string fmt_do = fmt::format("{{:{}>{}}}", fill_do, reps_do);

    fmt::print("\r[");
    if(reps_done > 0) {
        auto tmp_done = fmt::vformat(fmt_done, fmt::make_format_args(fill_done));
        fmt::print(fg(style.fg_done) | bg(style.bg_done) | fmt::emphasis::bold, "{}", tmp_done);
    }
    if(100 - reps_done > 0) {
        auto tmp_do = fmt::vformat(fmt_do, fmt::make_format_args(fill_do));
        fmt::print(fg(style.fg_do) | bg(style.bg_do), "{}", tmp_do);
    }
    fmt::print("]");
    fmt::print(fg(style.fg_percent) | bg(style.bg_percent) | fmt::emphasis::bold, " {:.2f}%", curr * scaling);
    fmt::print(fg(style.fg_message) | bg(style.bg_message), " {}", message);
    fflush(stdout);
}
