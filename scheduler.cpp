#include "scheduler.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <sstream>
#include <vector>


Scheduler::Scheduler(char* availability_filename, char* course_involvements_filename)
{
  std::string line;
  std::stringstream line_stream;
  std::string cell;
  // Read the availability file
  std::ifstream availability_f(availability_filename);
  // Read the header line
  std::getline(availability_f, line);
  line_stream << line;
  // Skip the first col
  std::getline(line_stream, cell, ',');
  // The remaining cols are the hour names
  while (std::getline(line_stream, cell, ','))
  {
    hour_names.push_back(cell);
  }
  line_stream.clear();
  // Read the remaining lines
  int person = 0;
  while (std::getline(availability_f, line))
  {
    availability.push_back(std::vector<bool>());
    line_stream << line;
    // Read the first column--the person name
    std::getline(line_stream, cell, ',');
    person_names.push_back(cell);
    // Read the remaining columns
    while (std::getline(line_stream, cell, ','))
    {
      availability.back().push_back(cell == "Y");
    }
    // If the line ends with a trailing comma, need to manually add a false element
    if (!line_stream && cell.empty())
    {
        availability.back().push_back(false);
    }
    person++;
    line_stream.clear();
  }

  // Read the course course involvements file
  std::ifstream course_involvements_f(course_involvements_filename);
  // Read the header line
  std::getline(course_involvements_f, line);
  line_stream << line;
  while (std::getline(line_stream, cell, ','))
  {
    course_names.push_back(cell);
    course_involvements.push_back(std::vector<int>());
  }
  line_stream.clear();
  // Read the remaining lines
  while (std::getline(course_involvements_f, line))
  {
    line_stream << line;
    int course = 0;
    while (std::getline(line_stream, cell, ','))
    {
      auto person = std::find(person_names.begin(), person_names.end(), cell) - person_names.begin();
      if (person < person_names.size())
      {
        course_involvements[course].push_back(person);
      }
      course++;
    }
    line_stream.clear();
  }
  // Output the parsed data. Useful for tracking the different inputs when
  // running the scheduler multiple times.
  std::cout << "Course involvements:" << std::endl;
  for (int r = 0; r < course_involvements.size(); r++) {
    std::cout << course_names[r] << ": ";
    for (int person : course_involvements[r])
    {
      std::cout << person_names[person] << ", ";
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  std::cout << "Person availability:" << std::endl;
  for (int person = 0; person < availability.size(); person++)
  {
    std::cout << std::setw(20) << person_names[person] << ": ";
    for (int hour = 0; hour < availability[person].size(); hour++)
    {
      if (availability[person][hour]) {
        std::cout << hour_names[hour] << ", ";
      }
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
}

void Scheduler::run()
{
  // hour -> courses
  std::vector<std::vector<int>> schedule(hour_names.size(), std::vector<int>());
  std::cout << "Starting scheduling!" << std::endl;
  // Kick off the recursive scheduling algorithm
  explore(schedule, availability, 0);
  std::cout << "Done scheduling. Found " << valid_schedules.size() << " solutions." << std::endl;
  if (valid_schedules.size() > 0)
  {
    std::cout << "Finding top solutions." << std::endl;
  }
  else
  {
    std::cout << "No solutions found." << std::endl;
  }
  find_best_schedules();
}

void Scheduler::explore(std::vector<std::vector<int>>& schedule, std::vector<std::vector<bool>>& availability, int depth)
{
  // Recursively try scheduling all the courses, in order, at all possible
  // times. The completion time of this search, of course, can vary wildly. Due
  // to increased pruning, it finishes dramatically sooner when there are more
  // conflicts and thus fewer (often zero) solutions. Directing the search by
  // scheduling the harder-to-schedule courses (less teacher availability, more
  // students/grade levels covered) first can lead to massive performance gains.
  for (int hour = 0; hour < hour_names.size(); hour++)
  {
    bool conflict = false;
    for (int person : course_involvements[depth])
    {
      if (!availability[person][hour])
      {
        conflict = true;
        break;
      }
    }
    // If there is a conflict, move onto the next hour
    if (conflict)
    {
      continue;
    }
    // Otherwise, the course can be scheduled at this hour, so schedule it.
    // Copies of the availability and the schedule must be used to prevent
    // contaminating them for the rest of the hours that this course could be
    // scheduled.
    std::vector<std::vector<int>> new_schedule(schedule);
    std::vector<std::vector<bool>> new_availability(availability);
    new_schedule[hour].push_back(depth);
    // Make sure everyone involved in the course is no longer available at this
    // hour
    for (int person : course_involvements[depth]) {
      new_availability[person][hour] = false;
    }
    if (depth + 1 == course_names.size())
    {
      // If we make it here, this is a complete, valid schedule!
      valid_schedules.push_back(new_schedule);
      std::cout << "Found " << valid_schedules.size() << " solutions...\r" << std::flush;
      // All courses have been scheduled, so don't recurse deeper
      continue;
    }
    // Continue with the next course
    explore(new_schedule, new_availability, depth + 1);
  }
}

void Scheduler::find_best_schedules()
{
  // Find and display all the schedules with the lowest (best) score
  std::vector<double> scores(valid_schedules.size(), std::numeric_limits<double>::infinity());
  for (int i = 0; i < valid_schedules.size(); i++)
  {
    scores[i] = calc_score(valid_schedules[i]);
  }
  double best_score = *std::min_element(scores.begin(), scores.end());
  int n = 0;
  for (int i = 0; i < scores.size(); i++)
  {
    if (scores[i] == best_score)
    {
      std::cout << "Best schedule (score = " << best_score << ") no. " << n++ << ":";
      print_schedule(valid_schedules[i]);
    }
  }
}

double Scheduler::calc_score(std::vector<std::vector<int>>& schedule)
{
  // Figure out which hours each teacher is teaching
  std::map<std::string, std::vector<int>> teacher_times;
  for (int hour = 0; hour < schedule.size(); hour++)
  {
    for (int course : schedule[hour])
    {
      int teacher = course_involvements[course][0];
      teacher_times[person_names[teacher]].push_back(hour);
    }
  }
  // Calculate the number of hours that each teacher spends waiting in between
  // classes and increment the score accordingly. Lower score is better.
  double score = 0;
  for (auto const& [teacher, times] : teacher_times)
  {
    // Which hour is this teacher's first/last class?
    int start = *std::min_element(times.begin(), times.end());
    int end = *std::max_element(times.begin(), times.end());
    // How long do they wait in between classes?
    score += end - start - times.size() + 1;
  }
  return score;
}

void Scheduler::print_schedule(const std::vector<std::vector<int>>& schedule)
{
  std::cout << std::endl;
  for (int hour = 0; hour < schedule.size(); hour++)
  {
    std::cout << std::setw(20) << hour_names[hour] << ": ";
    for (int course : schedule[hour])
    {
      std::cout << course_names[course] << " ";
    }
    std::cout << std::endl;
  }
  std::cout << "------------------" << std::endl;
}

int main(int argc, char* argv[])
{
  Scheduler scheduler = Scheduler(argv[1], argv[2]);
  scheduler.run();
  return 0;
}
