#ifndef __SPORTS_SCHEDULING_SCHEDULE_HPP__
#define __SPORTS_SCHEDULING_SCHEDULE_HPP__

/* schedule.hpp
 * Richard Gibson, May 14, 2015
 * Email: richard.g.gibson@gmail.com
 *
 * Class for generating and querying a sports schedule.
 *
 * Copyright (C) 2015 Richard Gibson
 */

/* C / C++ includes */
#include <array> // std::array
#include <vector> // std::vector
#include <stdio.h> // FILE

/* sports-scheduling includes */

typedef struct {
  int num_divisions;
  int num_teams_per_division;
  int num_games_vs_division;
  int num_games_vs_non_division;
  int seed;
} ss_schedule_params_t;

#define AWAY 0
#define HOME 1

#define BYE -1

typedef struct {
  std::array<int,2> teams;
} ss_game_t;

class SsSchedule {
public:
  /* Constructor - generates and stores the schedule */
  SsSchedule( const ss_schedule_params_t &params );

  /* Returns the number of days in the schedule */
  size_t num_days( ) const { return matches.size( ); }

  /* Returns the games for day 'day' */
  const std::vector<ss_game_t> &get_matches( const size_t day ) const
  { return matches[ day ]; }

  /* Prints out the schedule in a human-readable format to the given file */
  /* Note that teams are stored as 0-based but printed as 1-based */
  void print( FILE *file ) const;

  /* Prints out stats listing how many home and away games each team plays
   * to the given file
   */
  void print_stats( FILE *file ) const;

private:
  const ss_schedule_params_t params;
  /* matches[ day ] contains the matches for day 'day' */
  std::vector< std::vector<ss_game_t> > matches;
};

#endif
