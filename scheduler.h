#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <string>
#include <vector>


class Scheduler
{
public:
  Scheduler(char* availability_filename, char* course_involvements_filename);
  void run();
private:
  // person -> person name
  std::vector<std::string> person_names;
  // course -> course name
  std::vector<std::string> course_names;
  // hour -> hour name
  std::vector<std::string> hour_names;
  // person, hour -> available?
  std::vector<std::vector<bool>> availability;
  // course -> persons
  std::vector<std::vector<int>> course_involvements;
  // schedule no. -> hour -> course
  std::vector<std::vector<std::vector<int>>> valid_schedules;

  void explore(std::vector<std::vector<int>>& schedule, std::vector<std::vector<bool>>& availability, int depth);
  void find_best_schedules();
  double calc_score(std::vector<std::vector<int>>& schedule);
  void print_schedule(const std::vector<std::vector<int>>& schedule);
};

#endif // SCHEDULER_H_
