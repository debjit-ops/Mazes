#include <unistd.h>
#include "solver.h"
#include "cell.h"
#include "maze.h"
#include <cstdio>
#include <ncurses.h>
#include <vector>
#include <queue>

Solver::Solver(Maze* maze)
{
  this->maze = maze;
  grid = maze->get_grid();
  find_start_and_end();
}

void Solver::find_start_and_end()
{
  int max_row = maze->get_rows();
  int max_col = maze->get_cols();
  int found = 0;
  for (int i = 0; i < max_row && found < 2; i++)
  {
    for (int j = 0; j < max_col && found < 2; j++)
    {
      if (grid[i][j] == 'S')
      {
        start.row = i;
        start.col = j;
        found++;
      }
      if (grid[i][j] == 'E')
      {
        end.row = i;
        end.col = j;
        found++;
      }
    }
  }
}

bool Solver::backtrack(bool animate)
{
  bool ret_val;

  if (animate)
  {
    maze->init_curses();
    clear();
  }

  ret_val = backtrack_r(start.row, start.col, animate);

  endwin();

  return ret_val;
}

bool Solver::backtrack_r(int row, int col, bool animate)
{
  int dir = 0;
  do
  {
    int r = row;
    int c = col;

    get_new_cell(r, c, dir);

    if (is_valid(r, c))
    {
      if (grid[r][c] == 'E')
        return true;

      grid[r][c] = '*';

      if (animate)
      {
        maze->draw();
        usleep(DRAW_DELAY);
      }

      grid[r][c] = '.';

      if (backtrack_r(r, c, animate))
      {
        return true;
      }
      else
      {
        grid[r][c] = '*';
        dir++;

        if (animate)
        {
          maze->draw();
          usleep(DRAW_DELAY);
        }
        grid[r][c] = ' ';
      }
    }
    else
    {
      dir++;
    }
  } while (dir < 4);
  return false;
}

void Solver::get_new_cell(int& row, int& col, int dir)
{
  switch(dir)
  {
    case NORTH:
      row--;
      break;
    case EAST:
      col++;
      break;
    case SOUTH:
      row++;
      break;
    case WEST:
      col--;
      break;
  }
}

bool Solver::is_valid(int row, int col)
{
  bool retval = true;

  if (maze->is_valid(row, col))
  {
    switch (grid[row][col])
    {
      case '.':
      case '#':
      case 'S':
        retval = false;
    }
  }
  else
  {
    retval = false;
  }

  return retval;
}

void Solver::breadth_first_search(bool animate)
{
  std::queue<Cell*> frontier;
  std::queue<Cell*> to_delete;

  if (animate)
  {
    maze->init_curses();
    clear();
  }

  start.parent = NULL;
  frontier.push(&start);

  Cell* u;
  while (!frontier.empty())
  {
    u = frontier.front(); frontier.pop();
    grid[u->row][u->col] = '.';
    if (u->row == end.row && u->col == end.col)
    {
      for (Cell* runner = u; runner; runner = runner->parent)
      {
        grid[runner->row][runner->col] = '*';
        if (animate)
        {
          maze->draw();
          usleep(DRAW_DELAY);
        }
      }

      delete u;
      while(!frontier.empty())
      {
        u = frontier.front(); frontier.pop();
        delete u;
      }
      while (!to_delete.empty())
      {
        u = to_delete.front(); to_delete.pop();
        delete u;
      }

      getchar();
      endwin();
      return;
    }

    std::vector<Cell> neighbors = get_neighbors(u, maze->get_rows(), maze->get_cols());
    for (size_t i = 0; i < neighbors.size(); i++)
    {
      switch (grid[neighbors[i].row][neighbors[i].col])
      {
        case '.':
        case '#':
        case ',':
        case 'S':
          continue;
      }
      grid[neighbors[i].row][neighbors[i].col] = ',';
      if (animate)
      {
        maze->draw();
        usleep(DRAW_DELAY);
      }
      neighbors[i].parent = u;
      Cell* v = new Cell;
      *v = neighbors[i];
      frontier.push(v);
    }

    if (u->parent != NULL)
      to_delete.push(u);
  }

  getchar();
  endwin();

  return;
}

std::vector<Cell> get_neighbors(Cell* cell, const int max_row, const int max_col)
{
  std::vector<Cell> neighbors;
  Cell new_cell;
  int new_row;
  int new_col;
  int offset[4][2] = {{-1, 0},
    {0, +1},
    {+1, 0},
    {0, -1}};

  for (int i = 0; i < 4; i++)
  {
    new_row = cell->row + offset[i][0];
    new_col = cell->col + offset[i][1];
    if (new_row >= 0 && new_col >= 0 &&
        new_row < max_row && new_col < max_col)
    {
      new_cell.row = new_row;
      new_cell.col = new_col;
      neighbors.push_back(new_cell);
    }
  }

  return neighbors;
}
