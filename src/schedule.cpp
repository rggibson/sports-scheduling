/* schedule.cpp
 * Richard Gibson, May 14, 2015
 * Email: richard.g.gibson@gmail.com
 *
 * Class for generating and querying a sports schedule.
 *
 * Copyright (C) 2015 Richard Gibson
 */

/* C / C++ includes */
#include <stdexcept> // std::runtime_error
#include <assert.h> // assert
#include <chrono> // std::chrono::system_clock
#include <random> // std::default_random_engine
#include <algorithm> // shuffle

/* sports-scheduling includes */
#include "schedule.hpp"

SsSchedule::SsSchedule( const ss_schedule_params_t &params )
  : params( params )
{
  /* Check for problems */
  if( params.num_divisions > 2 ) {
    throw std::runtime_error( "Sorry, this library currently supports up to "
	      "a maximum of 2 divisions\n" );
  }

  /* Indices to track how many inter-division games we get through in the
   * intra-division games
   */
  int inter_round = 0;
  int inter_match_idx = 0;

  /* Generate vs division games first */
  for( int round = 0; round < params.num_games_vs_division; ++round ) {

    /* We'll handle a few cases separately */
    if( params.num_teams_per_division % 2 == 0 ) {
      /* Even number of teams per division */

      /* This is the easiest case, simply
       * apply the well-known round robin tournament algorithm:
       * http://en.wikipedia.org/wiki/
       * Round-robin_tournament#Scheduling_algorithm
       */
      std::vector<ss_game_t> this_matches
	( params.num_teams_per_division * params.num_divisions / 2 );
      for( int match_idx = 1; match_idx < params.num_teams_per_division;
	   ++match_idx ) {

	int game_num = 0;
	for( int div = 0; div < params.num_divisions; ++div ) {
	  /* Teams for this division are in the
	   * [offset, offset + params.num_teams_per_division) range,
	   * where offset is defined below
	   */
	  const int offset = div * params.num_teams_per_division;

	  /* Handle team offset's game first */
	  int team1 = offset;
	  int team2 = offset + match_idx;
	  
	  /* To get equal number of home / away games, we do the following:
	   * flag = round + team1 + team2 % 2, where team1 < team2.
	   * Then team1 is away if and only if flag == 0.
	   */
	  int flag = ( round + team1 + team2 ) % 2;
	  this_matches[ game_num ].teams[ flag ] = team1;
	  this_matches[ game_num ].teams[ 1 - flag ] = team2;
	  ++game_num;

	  /* Now loop over all other games for this division */
	  team1 = team2;
	  for( int i = 1; i < params.num_teams_per_division / 2; ++i ) {
	    /* Update teams */
	    --team1;
	    if( team1 <= offset ) {
	      team1 = offset + params.num_teams_per_division - 1;
	    }
	    ++team2;
	    if( team2 >= offset + params.num_teams_per_division ) {
	      team2 = offset + 1;
	    }

	    /* Set game */
	    flag = ( round + team1 + team2 ) % 2;
	    this_matches[ game_num ].teams[ flag ]
	      = ( team1 < team2 ? team1 : team2 );
	    this_matches[ game_num ].teams[ 1 - flag ]
	      = ( team1 < team2 ? team2 : team1 );
	    ++game_num;
	  }
	}
	assert( static_cast<uint32_t>( game_num ) == this_matches.size( ) );

	/* Finally, save the games for this day */
	matches.push_back( this_matches );
      }

    } else {
      /* Odd number of teams per division */

      /* Typically when we have odd number of teams in two divisions,
       * we can avoid byes
       * by having one team from each division play each other.
       * This is true as long as there are enough games vs other
       * division to do this.
       */

      /* Start by determining whether or not we can avoid byes */
      assert( params.num_divisions <= 2 );
      const bool do_byes
	= ( ( params.num_divisions == 1 )
	    || ( inter_round >= params.num_games_vs_non_division ) );

      std::vector<ss_game_t> this_matches
	( params.num_teams_per_division * params.num_divisions / 2
	  + ( do_byes ? 1 : 0 ) );
      for( int match_idx = 1; match_idx < params.num_teams_per_division + 1;
	   ++match_idx ) {
	uint32_t game_num = 0;

	int teamA;
	int teamB;
	if( !do_byes ) {
	  /* Handle the inter-division match first */
	  assert( params.num_divisions == 2 );
	  teamA = match_idx - 1;
	  teamB = ( ( match_idx - 1 + inter_match_idx )
		    % params.num_teams_per_division )
	    + params.num_teams_per_division;
	  const int flag = ( teamA + teamB + inter_round ) % 2;
	  if( teamA != 0 ) {
	    /* Always put team 0's game at the front */
	    game_num = this_matches.size( ) - 1;
	  }
	  this_matches[ game_num ].teams[ flag ] = teamA;
	  this_matches[ game_num ].teams[ 1 - flag ] = teamB;
	  if( teamA == 0 ) {
	    game_num = 1;
	  } else {
	    game_num = 0;
	  }
	}

	for( int div = 0; div < params.num_divisions; ++div ) {
	  /* Teams for this division are in the
	   * [offset, offset + params.num_teams_per_division) range,
	   * where offset is defined below
	   */
	  const int offset = div * params.num_teams_per_division;

	  /* This int represents a "bye" team */
	  const int bye_team = offset + params.num_teams_per_division;

	  int team1, team2;
	  if( do_byes ) {
	    /* Handle team offset's game first */
	    team1 = offset;
	    team2 = offset + match_idx;
	  
	    if( team2 == bye_team ) {
	      this_matches[ game_num ].teams[ AWAY ] = team1;
	      this_matches[ game_num ].teams[ HOME ] = BYE;
	    } else {
	      const int flag = ( round + team1 + team2 ) % 2;
	      this_matches[ game_num ].teams[ flag ] = team1;
	      this_matches[ game_num ].teams[ 1 - flag ] = team2;
	    }
	    ++game_num;
	  } else if( div == 0 ) {
	    team2 = teamA;
	  } else if( div == 1 ) {
	    team2 = teamB;
	  } else {
	    throw std::runtime_error( "Too many divisions" );
	  }
	  
	  /* Now loop over all other games for this division */
	  team1 = team2;
	  for( int i = 1; i < ( params.num_teams_per_division + 1 ) / 2;
	       ++i ) {
	    /* Update teams */
	    --team1;
	    if( ( team1 <= offset && do_byes ) || team1 < offset ) {
	      team1 = bye_team - ( do_byes ? 0 : 1 );
	    }
	    ++team2;
	    if( ( team2 >= bye_team && !do_byes ) || team2 > bye_team ) {
	      team2 = offset + ( do_byes ? 1 : 0 );
	    }

	    /* Set game */
	    if( team1 == bye_team ) {
	      this_matches[ game_num ].teams[ AWAY ] = team2;
	      this_matches[ game_num ].teams[ HOME ] = BYE;
	    } else if( team2 == bye_team ) {
	      this_matches[ game_num ].teams[ AWAY ] = team1;
	      this_matches[ game_num ].teams[ HOME ] = BYE;
	    } else {
	      const int flag = ( round + team1 + team2 ) % 2;
	      this_matches[ game_num ].teams[ flag ]
		= ( team1 < team2 ? team1 : team2 );
	      this_matches[ game_num ].teams[ 1 - flag ]
		= ( team1 < team2 ? team2 : team1 );
	    }
	    ++game_num;
	  }
	}
	if( !do_byes && match_idx > 1 ) {
	  assert( game_num == this_matches.size( ) - 1 );
	} else {
	  assert( game_num == this_matches.size( ) );
	}

	/* Finally, save the games for this day */
	matches.push_back( this_matches );

      }

      if( !do_byes ) {
	/* Increment inter_match_idx */
	++inter_match_idx;
	if( inter_match_idx >= params.num_teams_per_division ) {
	  ++inter_round;
	  inter_match_idx = 0;
	}
      }
    }
  }

  /* Generate vs non division games next */
  const int num_games_vs_non_division
    = ( params.num_divisions > 1 ? params.num_games_vs_non_division : 0 );
  for( ; inter_round < num_games_vs_non_division;
       ++inter_round, inter_match_idx = 0 ) {
    for( ; inter_match_idx < params.num_teams_per_division;
	 ++inter_match_idx ) {
      /* Simply match up the i'th team from one division with the
       * ( i + match_idx mod num_teams )'th team from the other division
       */
      std::vector<ss_game_t> this_matches( params.num_teams_per_division );
      for( int i = 0; i < params.num_teams_per_division; ++i ) {
	const int team1 = i;
	const int team2
	  = ( ( i + inter_match_idx ) % params.num_teams_per_division )
	  + params.num_teams_per_division;

	const int flag = ( inter_round + team1 + team2 ) % 2;
	this_matches[ i ].teams[ flag ] = team1;
	this_matches[ i ].teams[ 1 - flag ] = team2;
      }
      matches.push_back( this_matches );
    }
  }

  /* Randomize order of days */
  unsigned seed;
  if( params.seed >= 0 ) {
    seed = params.seed;
  } else {
    /* Obtain a time-based seed */
    seed = std::chrono::system_clock::now( ).time_since_epoch( ).count( );
  }
  shuffle( matches.begin( ), matches.end( ),
	   std::default_random_engine( seed ) );
}

void SsSchedule::print( FILE *file ) const
{
  fprintf( file, "TEAMS:\n" );
  for( int div = 0; div < params.num_divisions; ++div ) {
    fprintf( file, "  DIV %c:", 'A' + div );
    const int offset = div * params.num_teams_per_division;
    for( int i = 0; i < params.num_teams_per_division; ++i ) {
      fprintf( file, " %d", offset + i + 1 );
    }
    fprintf( file, "\n" );
  }
  fprintf( file, "\n" );

  for( size_t d = 0; d < matches.size( ); ++d ) {
    const std::vector<ss_game_t> &day_matches = matches[ d ];
    fprintf( file, "DAY %zu:\n", d + 1 );
    for( const ss_game_t &game : day_matches ) {
      if( game.teams[ AWAY ] == BYE ) {
	fprintf( file, "BYE" );
      } else {
	fprintf( file, "%d", game.teams[ AWAY ] + 1 );
      }
      fprintf( file, " at " );      
      if( game.teams[ HOME ] == BYE ) {
	fprintf( file, "BYE\n" );
      } else {
	fprintf( file, "%d\n", game.teams[ HOME ] + 1 );
      }
    }
    fprintf( file, "\n" );
  }
}

void SsSchedule::print_stats( FILE *file ) const
{
  /* Calculate stats */
  const int num_teams = params.num_divisions * params.num_teams_per_division;
  std::vector< std::vector<int> > num_games_crosstable
    ( num_teams, std::vector<int>( num_teams, 0 ) );
  std::vector< std::vector<int> > num_opp_games_crosstable( num_teams );
  for( int i = 0; i < num_teams; ++i ) {
    num_opp_games_crosstable[ i ].assign( i, 0 );
  }
  std::vector< std::vector<int> > total_num_games_by_team
    ( num_teams, std::vector<int>( 2, 0 ) );
  std::vector<bool> team_plays( num_teams );
  for( const std::vector<ss_game_t> &games : matches ) {
    std::fill( team_plays.begin( ), team_plays.end( ), false );
    for( const ss_game_t &game : games ) {
      if( game.teams[ AWAY ] != BYE && game.teams[ HOME ] != BYE ) {
	num_games_crosstable[ game.teams[ AWAY ] ][ game.teams[ HOME ] ] += 1;
	if( game.teams[ AWAY ] < game.teams[ HOME ] ) {
	  num_opp_games_crosstable[ game.teams[ HOME ] ][ game.teams[ AWAY ] ]
	    += 1;
	} else {
	  num_opp_games_crosstable[ game.teams[ AWAY ] ][ game.teams[ HOME ] ]
	    += 1;
	}
	total_num_games_by_team[ game.teams[ AWAY ] ][ AWAY ] += 1;
	total_num_games_by_team[ game.teams[ HOME ] ][ HOME ] += 1;
	assert( !team_plays[ game.teams[ AWAY ] ] );
	team_plays[ game.teams[ AWAY ] ] = true;
	assert( !team_plays[ game.teams[ HOME ] ] );
	team_plays[ game.teams[ HOME ] ] = true;
      }
    }
  }

  /* Print stats */
  fprintf( file, "########## Schedule Stats ##########\n\n" );
  fprintf( file, "NUM AWAY \\ HOME GAMES VS OPPONENT CROSSTABLE:\n" );
  fprintf( file, "A\\H" );
  for( int i = 0; i < num_teams; ++i ) {
    fprintf( file, " %d", i + 1 );
  }
  fprintf( file, "\n" );
  for( int i = 0; i < num_teams; ++i ) {
    fprintf( file, " %d ", i + 1 );
    for( int j = 0; j < num_teams; ++j ) {
      fprintf( file, " %d", num_games_crosstable[ i ][ j ] );
    }
    fprintf( file, "\n" );
  }
  fprintf( file, "\n" );
  fprintf( file, "NUM GAMES VS OPPONENT HALF-CROSSTABLE:\n" );
  fprintf( file, " " );
  for( int i = 0; i < num_teams; ++i ) {
    fprintf( file, " %d", i + 1 );
  }
  fprintf( file, "\n" );
  for( int i = 0; i < num_teams; ++i ) {
    fprintf( file, "%d", i + 1 );
    for( int j = 0; j < i; ++j ) {
      fprintf( file, " %d", num_opp_games_crosstable[ i ][ j ] );
    }
    fprintf( file, "\n" );
  }
  fprintf( file, "\n" );
  for( int i = 0; i < num_teams; ++i ) {
    fprintf( file, "Team %d plays %d away games and %d home games\n",
	     i + 1, total_num_games_by_team[ i ][ AWAY ],
	     total_num_games_by_team[ i ][ HOME ] );
  }
  fprintf( file, "%zu days of games\n", matches.size( ) );
}
