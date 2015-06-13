/* test.cpp
 * Richard Gibson, May 14, 2015
 * Email: richard.g.gibson@gmail.com
 *
 * Program for testing construction of sports schedules.
 *
 * Copyright (C) 2015 Richard Gibson
 */

/* C / C++ includes */
#include <stdlib.h> // atoi
#include <string.h> // strlen
#include <stdexcept> // std::runtime_error
#include <iostream> // cerr

/* sports-scheduling includes */
#include "schedule.hpp"

int main( const int argc, const char *argv[] )
{
  ss_schedule_params_t params;
  params.seed = -1;

  /* Check for correct number of arguments */
  if( argc < 5 ) {
    fprintf( stderr, "Usage: %s <num_divisions> <num_teams_per_division> "
	     "<num_games_vs_division> <num_games_vs_non_division> [options]\n",
	     argv[ 0 ] );
    fprintf( stderr, "Options:\n" );
    fprintf( stderr, "  --rng={<seed>|TIME} (default: TIME)\n" );
    return 1;
  }

  /* Parse command line */
  int index = 1;

  /* Number of divisions */
  params.num_divisions = atoi( argv[ index ] );
  if( params.num_divisions <= 0 ) {
    fprintf( stderr, "Failed to parse number of divisions from [%s]\n",
	     argv[ index ] );
    return 1;
  }
  ++index;

  /* Number of teams per division */
  params.num_teams_per_division = atoi( argv[ index ] );
  if( params.num_teams_per_division <= 0 ) {
    fprintf( stderr, "Failed to parse number of teams per division from [%s]"
	     "\n", argv[ index ] );
    return 1;
  }
  ++index;
  
  /* Number of games vs division opponents */
  params.num_games_vs_division = atoi( argv[ index ] );
  if( params.num_games_vs_division < 0 ) {
    fprintf( stderr, "Failed to parse number of games vs division opponents "
	     "from [%s]\n", argv[ index ] );
    return 1;
  }
  ++index;

  /* Number of games vs non-division opponents */
  params.num_games_vs_non_division = atoi( argv[ index ] );
  if( params.num_games_vs_non_division < 0 ) {
    fprintf( stderr, "Failed to parse number of games vs non-division "
	     "opponents from [%s]\n", argv[ index ] );
    return 1;
  }
  ++index;

  /* Parse options */
  for( ; index < argc; ++index ) {
    if( !strncmp( argv[ index ], "--rng=", strlen( "--rng=" ) ) ) {
      if( !strcmp( argv[ index ], "--rng=TIME" ) ) {
	params.seed = -1;
      } else {
	const int num_scanned = sscanf( argv[ index ], "--rng=%d",
					&params.seed );
	if( num_scanned < 1 ) {
	  fprintf( stderr, "Failed to parse rng seed from [%s]\n",
		   argv[ index ] );
	  return 1;
	}
	if( params.seed < 0 ) {
	  fprintf( stderr, "Sorry, rng seed must be a positive integer, but "
		   "received %d\n", params.seed );
	  return 1;
	}
      }

    } else {
      fprintf( stderr, "Failed to parse command line option [%s]\n",
	       argv[ index ] );
      return 1;
    }
  }

  /* Finished parsing the command line */
  
  try {
    /* Generate schedule */
    const SsSchedule schedule( params );

    /* Print schedule */
    schedule.print( stdout );

    /* Print stats */
    schedule.print_stats( stderr );

  } catch( const std::runtime_error &re ) {
    std::cerr << "Runtime error: " << re.what( ) << std::endl;
    return 1;
  }

  /* Done! */
  return 0;
}
