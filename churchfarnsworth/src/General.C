/*
 * General.C
 *
 *  Created on: Nov 17, 2009
 *      Author: mfchurch
 */

#include "General.H"

General::General() {
	// TODO Auto-generated constructor stub

}

General::~General() {
	// TODO Auto-generated destructor stub
}


//General Loop
/*
 * for each tile in grid:
  if tile contains marine:
    if friendly:
      tile.risk -= 1
    else:
      tile.risk += 1
  if tile contains tank:
    if friendly:
      if deployed:
        tile.risk -= 5
      else:
        tile.risk -= 3
    else:
      if deployed:
        tile.risk += 5
      else:
        tile.risk += 3
depreciate tile value
 */
