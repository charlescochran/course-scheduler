# Course Scheduler

C++ recursive course scheduler for small school/co-op environments.

Input: When is everyone available? What courses is everyone involved with?
Output: All possible schedules, but paired down with some filtering (see
below).

## How do I use it?

### Make a Google sheet

See [here](https://docs.google.com/spreadsheets/d/19k_7FAme3dM47XDbonsZkjBLbnX8j8WZGeEaVBk6mWw/edit?usp=sharing)
for an example.

Unless you want to manually make the CSV file used below (not recommended),
make a Google sheet with the input data.
- In the first sheet ("availability"), the first column contains each person's
  name, while remaining columns correspond to the timeslots in consideration.
  A "Y" indicates the given person's availability during the given timeslot;
  anything else (include nothing) indicates inavailability. Note that the
  first name in each column is considered the course's teacher; this has no
  meaning until the schedule ranking step (see below).
- In the second sheet ("course_involvements"), the columns correspond to
  courses. Under each course's name is a list of the people involved in (i.e.,
  enrolled or teaching) that course. These names must correspond exactly to
  those in the first sheet.
Note: spaces in the course/person names are fine, but commas are not.

### Build, setup, and run

1. Clone this repo and compile with `g++ -O3 -o scheduler scheduler.cpp`.
2. Get the CSV files for each sheet (`File -> Download -> Comma Separated
   Values (.csv)`), move them to the directory with the `scheduler` binary,
   and convert the Windows-style line endings to Unix-style line endings (see
   `dos2unix`).
3. Run `./scheduler [availability_file].csv [course_involvements_file].csv`.

## What will it do?

1. Parse the input data (and output the parsed data).
2. Run a recursive DFS, trying to schedule each course at each timeslot (this
   can take a long time; see below). This will tend to produce either no
   solutions or a large number of them.
3. If there are multiple solutions, the program will rank them based on how
   well they minimize the amount of time that teachers must spend waiting in
   between classes (i.e., back-to-back teaching is preferred), assigning each
   solution a score. Lower scores are better. The best solutions, using this
   metric, will be displayed.

Here is the output with the simple example Google sheet referenced above:
```
Course involvements:
Course1: Dylan Bob, Musk Elon, Ford Harrison, Zuckerberg Mark,
Course2: Dylan Bob,
Course3: Musk Elon, Zuckerberg Mark,
Course4: Dylan Bob, Ford Harrison, Zuckerberg Mark,

Person availability:
           Dylan Bob: Time1, Time2, Time3, Time4,
           Musk Elon: Time3, Time5,
       Ford Harrison: Time2, Time3, Time4, Time5,
     Zuckerberg Mark: Time1, Time2, Time3, Time4, Time5,

Starting scheduling!
Done scheduling. Found 4 solutions.
Finding top solutions.
Best schedule (score = 0) no. 0:
               Time1: Course2
               Time2: Course4
               Time3: Course1
               Time4:
               Time5: Course3
------------------
Best schedule (score = 0) no. 1:
               Time1:
               Time2: Course2
               Time3: Course1
               Time4: Course4
               Time5: Course3
------------------
Best schedule (score = 0) no. 2:
               Time1:
               Time2: Course4
               Time3: Course1
               Time4: Course2
               Time5: Course3
------------------
```

## A note on speed

The DFS's run time can vary wildly due to the problems exponential nature. Due
to increased pruning, when there are fewer solutions (or no solutions), the
algorithm runs faster.

Directing the search can have a *huge* impact on performance. The DFS tries
scheduling courses in the order that they appear in the Google sheet. Putting
the courses which are harder to schedule/more likely to cause conflicts
(teachers are available less often, more students enrolled, more grade levels
covered, etc.) earlier in the input will cause the pruning to take effect
sooner, significantly reducing the number of function calls which must occur.

Note that the output indicates how many solutions have been found so far. If
there are any solutions, this number will probably start increasing early on.
If there are no solutions, nothing will be printed while running. This can be
a useful way of determining if there will be any solutions with the given
input without waiting for the DFS to actually finish.
