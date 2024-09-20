#ifndef YTABLE_H
#define YTABLE_H


#include <map>

class YTableCell {
public:
    YTableCell() {r1 = r2 = c1 = c2 = 0;}
    YTableCell(int r1, int r2, int c1, int c2) {
        this->r1 = r1;
        this->r2 = r2;
        this->c1 = c1;
        this->c2 = c2;
    }

    int rowspan() {return r2 - r1;}
    int colspan() {return c2 - c1;}
    int span() {return rowspan() * colspan();}

    bool contain(YTableCell cell) {
        if (cell.r1 >= r1 && cell.r2 <= r2 &&
            cell.c1 >= c1 && cell.c2 <= c2)
            return true;
        return false;
    }

    int r1, r2, c1, c2;
};

class YTable
{
public:
    YTable();
    void init(int row, int col);
    bool getTableCell(int id, YTableCell& rst);
    YTableCell merge(int lt, int rb);

public:
    int row;
    int col;
    std::map<int, YTableCell> m_mapCells;   // id => YTabelCell
};

#endif // YTABLE_H
