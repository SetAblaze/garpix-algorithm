#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/ostreamwrapper.h>

enum ECorners {
  C_LEFT_BACK,
  C_RIGHT_BACK,
  C_LEFT_FRONT,
  C_RIGHT_FRONT
};

inline int Max(int a, int b) {
  return a > b ? a : b;
};

inline int Min(int a, int b) {
  return a < b ? a : b;
};

//Координаты плоскостей контейнера
struct TBorders {
  int xLeft;
  int xRight; //xRight > xLeft
  int yFront;
  int yBack;//yFront > yBack
  int zTop;
  int zBottom;//zTop > zBottom
};

struct TCargoGroup {
  int Mass;
  int Width;
  int Height;
  int Length;
  int GroupId;
  int Sort;
  int Count;
};

//***********************************************************************************************************
class TContainer {
public:
  TContainer::TContainer(const TCargoGroup group)
    : Group(group)
  {
    Volume = Group.Width * Group.Height * Group.Length;
    IsPlaced = false;
  }

  bool TryToPlace(TBorders areaToPlace, std::vector<TBorders>& blankSpaces) { // Возвращает false, если разместить контейнер с невисящим центром массы не удалось
    return Place(areaToPlace, blankSpaces, C_LEFT_BACK)
      || Place(areaToPlace, blankSpaces, C_RIGHT_BACK)
      || Place(areaToPlace, blankSpaces, C_LEFT_FRONT)
      || Place(areaToPlace, blankSpaces, C_RIGHT_FRONT);
  }

  int GetVolume() {
    return Volume;
  }

  TBorders& GetBorders() {
    return Borders;
  }

  int GetMass() {
    return Group.Mass;
  }

  bool HasIntersection(TBorders& otherBorders) {
    if (!IsPlaced) {
      return false;
    }

    bool noXIntersection = (Borders.xLeft >= otherBorders.xRight) || (otherBorders.xLeft >= Borders.xRight);
    bool noYIntersection = (Borders.yBack >= otherBorders.yFront) || (otherBorders.yBack >= Borders.yFront);
    bool noZIntersection = (Borders.zBottom >= otherBorders.zTop) || (otherBorders.zBottom >= Borders.zTop);
    return !noXIntersection && !noYIntersection && !noZIntersection;
  }

protected:
  //Проверка пересечения контейнеров в Cargo Space
  bool Place(TBorders areaToPlace, std::vector<TBorders>& blankSpaces, ECorners corner) { // Возвращает false, если разместить контейнер с невисящим центром массы не удалось
    if ((areaToPlace.xRight - areaToPlace.xLeft) < Group.Width) {
      return false;
    };

    if ((areaToPlace.yFront - areaToPlace.yBack) < Group.Length) {
      return false;
    }

    if ((areaToPlace.zTop - areaToPlace.zBottom) < Group.Height) {
      return false;
    }

    float x = -1;
    float y = -1;
    if (corner == C_LEFT_BACK) {
      x = (float)areaToPlace.xLeft + (float)Group.Width / 2.;
      y = (float)areaToPlace.yBack + (float)Group.Length / 2.;
    } else if (corner == C_RIGHT_BACK) {
      x = (float)areaToPlace.xRight - (float)Group.Width / 2.;
      y = (float)areaToPlace.yBack + (float)Group.Length / 2.;
    } else if (corner == C_LEFT_FRONT) {
      x = (float)areaToPlace.xLeft + (float)Group.Width / 2.;
      y = (float)areaToPlace.yFront - (float)Group.Length / 2.;
    } else if (corner == C_RIGHT_FRONT) {
      x = (float)areaToPlace.xRight - (float)Group.Width / 2.;
      y = (float)areaToPlace.yFront - (float)Group.Length / 2.;
    }

    if (!CheckMassPosition(x, y, areaToPlace.zBottom, blankSpaces) || !CheckLocationAvailability()) {
      return false;
    }

    IsPlaced = true;
    Borders.zBottom = areaToPlace.zBottom;
    Borders.zTop = Borders.zBottom + Group.Height;
    if (corner == C_LEFT_BACK) {
      Borders.xLeft = areaToPlace.xLeft;
      Borders.xRight = Borders.xLeft + Group.Width;
      Borders.yBack = areaToPlace.yBack;
      Borders.yFront = Borders.yBack + Group.Length;
    }
    else if (corner == C_RIGHT_BACK) {
      Borders.xRight = areaToPlace.xRight;
      Borders.xLeft = Borders.xRight - Group.Width;
      Borders.yBack = areaToPlace.yBack;
      Borders.yFront = Borders.yBack + Group.Length;
    }
    else if (corner == C_LEFT_FRONT) {
      Borders.xLeft = areaToPlace.xLeft;
      Borders.xRight = Borders.xLeft + Group.Width;
      Borders.yFront = areaToPlace.yFront;
      Borders.yBack = Borders.yFront - Group.Length;
    }
    else if (corner == C_RIGHT_FRONT) {
      Borders.xRight = areaToPlace.xRight;
      Borders.xLeft = Borders.xRight - Group.Width;
      Borders.yFront = areaToPlace.yFront;
      Borders.yBack = Borders.yFront - Group.Length;
    }

    std::cout << " placed on level " << Borders.zBottom << " at point (" << Borders.xLeft << ", " << Borders.yBack << ")\n";

    return true;
  }

  bool CheckLocationAvailability() {
    return true; // TODO проверку доступности этого места
  }

  bool CheckMassPosition(float x, float y, int h, std::vector<TBorders>& places) {
    for (const auto& place : places) {
      if (place.zBottom < h
        && (float)place.xLeft <= x
        && (float)place.xRight >= x
        && (float)place.yBack <= y
        && (float)place.yFront >= y) {
        return false;
      }
    }

    return true;
  }

private:
    TCargoGroup Group;
    TBorders Borders;
    int Volume; //Объем
    bool IsPlaced;
};



//***********************************************************************************************************
class TCargoSpace {
public:
    //Конструктор
    TCargoSpace::TCargoSpace(const int width, const int height, const int length, const int mass, const int carrying_capacity)
        : Width(width)
        , Height(height)
        , Length(length)
        , Mass(mass)
        , CarryingCapacity(carrying_capacity)
        , CurrentLoadingWeight(0)
    {
      TBorders IBox;
      IBox.xLeft = 0;
      IBox.xRight = width;
      IBox.yFront = length;
      IBox.yBack = 0;
      IBox.zTop = height;
      IBox.zBottom = 0;
      PotentialContainers.push_back(std::move(IBox));
      CurrentBlankVolume = width * height * length;
    }

    TCargoSpace::~TCargoSpace() {
		for (int i = 0; i < LoadedSpace.size(); i++) {
			delete LoadedSpace[i];
			LoadedSpace[i] = nullptr;
		}
    }


    std::vector<TCargoGroup>& LoadContainers(std::vector<TCargoGroup>& inputStream) { // Возвращает контейнеры, которые не поместились.
      std::vector<TCargoGroup> result;

      int i = 0;
      for (int i = 0; i < inputStream.size(); i++) {
        std::cout << "Container " << i << "  " << inputStream[i].Width << " x " << inputStream[i].Length << " x " << inputStream[i].Height;
        if (!PlaceContainer(inputStream[i])) {
          result.push_back(inputStream[i]);
          std::cout << " not placed\n";
        }
      }

      return result;
    }

    int GetMass() {
      return Mass;
    }
    int GetWidth() {
      return Width;
    }
    int GetHeight() {
      return Height;
    }
    int GetLength() {
      return Length;
    }
    int GetCarryingCapacity() {
      return CarryingCapacity;
    }

protected:

    bool PlaceContainer(TCargoGroup& a) { // Возвращает false, если место кончилось
        if (a.Mass > CarryingCapacity - CurrentLoadingWeight) {
            return false;
        }

        int volume = a.Width * a.Height * a.Length;
        if (volume > CurrentBlankVolume) {
            return false;
        }

        if (!LoadConteiner(a)) {
            return false;
        }

        CurrentLoadingWeight += a.Mass;
        CurrentBlankVolume -= volume;
        RebuildPotentialContainers();
        return true;
    }


    bool LoadConteiner(TCargoGroup& a) {
      bool result = false;
      TContainer* newContainer = new TContainer(a);
      for (const auto& potential : PotentialContainers) {
        if (newContainer->TryToPlace(potential, PotentialContainers)) {
          LoadedSpace.push_back(newContainer);
          result = true;
          break;
        }
      }

      if (!result) {
        delete newContainer;
      }

      return result;
    }

    void RebuildPotentialContainers() {
      // Рисуем сеточку на основе координат уже размещенных контейнеров
      std::vector<int> xLines;
      std::vector<int> yLines;
      std::vector<int> zLines;
      xLines.push_back(Width);
      xLines.push_back(0);
      yLines.push_back(0);
      yLines.push_back(Length);
      zLines.push_back(0);
      zLines.push_back(Height);

      for (const auto box : LoadedSpace) {
        const auto params = box->GetBorders();
        xLines.push_back(params.xLeft);
        xLines.push_back(params.xRight);
        yLines.push_back(params.yBack);
        yLines.push_back(params.yFront);
        zLines.push_back(params.zBottom);
        zLines.push_back(params.zTop);
      }

      std::sort(xLines.begin(), xLines.end());
      std::sort(yLines.begin(), yLines.end());
      std::sort(zLines.begin(), zLines.end());

      // Строим базовые потенциальные контейнеры
      std::vector<TBorders> elements;
      int prev_x1 = -1;
      for (int i1 = 0; i1 < xLines.size() - 1; i1++) {
        if (prev_x1 == xLines[i1])
          continue;
        else
          prev_x1 = xLines[i1];
        for (int i2 = i1 + 1; i2 < xLines.size(); i2++) {
          int prev_x2 = -1;
          if (prev_x2 == xLines[i2])
            continue;
          else
            prev_x2 = xLines[i2];
          int prev_y1 = -1;
          for (int j1 = 0; j1 < yLines.size() - 1; j1++) {
            if (prev_y1 == yLines[j1])
              continue;
            else
              prev_y1 = yLines[j1];
            int prev_y2 = -1;
            for (int j2 = j1 + 1; j2 < yLines.size(); j2++) {
              if (prev_y2 == yLines[j2])
                continue;
              else
                prev_y2 = yLines[j2];
              int prev_z1 = -1;
              for (int k1 = 0; k1 < zLines.size() - 1; k1++) {
                if (prev_z1 == zLines[k1])
                  continue;
                else
                  prev_z1 = zLines[k1];
                int prev_z2 = -1;
                for (int k2 = k1 + 1; k2 < zLines.size(); k2++) {
                  if (prev_z2 == zLines[k2])
                    continue;
                  else
                    prev_z2 = zLines[k2];
                  if (xLines[i1] != xLines[i2] && yLines[j1] != yLines[j2] && zLines[k1] != zLines[k2]) {
                    TBorders newBox;
                    newBox.xLeft = Min(xLines[i1], xLines[i2]);
                    newBox.xRight = Max(xLines[i1], xLines[i2]);
                    newBox.yFront = Max(yLines[j1], yLines[j2]);
                    newBox.yBack = Min(yLines[j1], yLines[j2]);
                    newBox.zTop = Max(zLines[k1], zLines[k2]);
                    newBox.zBottom = Min(zLines[k1], zLines[k2]);
                    if (CheckLoadedIntersection(newBox)) {
                      elements.push_back(std::move(newBox));
                    }
                  }
                }
              }
            }
          }
        }
      }

      // Оставляем только самые большие (которые никуда нельзя включить)
      PotentialContainers.clear();
      for (int i = 0; i < elements.size(); i++) {
        bool isMaximal = true;
        for (int j = 0; j < elements.size(); j++) {
          if (i == j)
            continue;

          if (FirstIncludesSecond(elements[j], elements[i])) {
            isMaximal = false;
            break;
          }
        }

        if (isMaximal) {
          bool isEqual = false;
////          for (const auto& old_element : PotentialContainers) {
////            if FirstEqualSecond(elements[i], old_element)
////          }
          PotentialContainers.push_back(std::move(elements[i]));
////          std::cout << "container " << elements[i].xLeft << "-" << elements[i].xRight << ", " << elements[i].yBack << "-" << elements[i].yFront << ", " << elements[i].zBottom << "-" << elements[i].zTop << "\n";
        }
      }

      SortPotential();
    }

    bool FirstIncludesSecond(TBorders& box1, TBorders& box2) {
      bool result = (box1.xRight - box1.xLeft) > (box2.xRight - box2.xLeft)
        || (box1.yFront - box1.yBack) > (box2.yFront - box2.yBack)
        || (box1.zTop - box1.zBottom) > (box2.zTop - box2.zBottom);

      result = result
        && box1.xRight >= box2.xRight
        && box1.xLeft <= box2.xLeft
        && box1.yFront >= box2.yFront
        && box1.yBack <= box2.yBack
        && box1.zTop >= box2.zTop
        && box1.zBottom <= box2.zBottom;

      return result;
    }

    bool FirstEqualSecond(TBorders& box1, TBorders& box2) {
      bool result = box1.xRight == box2.xRight
        && box1.xLeft == box2.xLeft
        && box1.yFront == box2.yFront
        && box1.yBack == box2.yBack
        && box1.zTop == box2.zTop
        && box1.zBottom == box2.zBottom;

      return result;
    }

    bool CheckLoadedIntersection(TBorders& box) {
      for (const auto& container : LoadedSpace) {
        if (container->HasIntersection(box)) {
          return false;
        }
      }

      return true;
    }

   // bool comp(TBorders a, TBorders b) {
    //   return a.xLeft < b.xLeft;
    //}
   struct comp
    {
      inline bool operator() (TBorders a, TBorders b)
      {
        return (a.xRight - a.xLeft) * (a.zTop - a.zBottom) * (a.yFront - a.yBack)
          < (b.xRight - b.xLeft) * (b.zTop - b.zBottom) * (b.yFront - b.yBack);
      }
    };

    //Сортировка потенциальных контейнеров по убыванию
    void SortPotential() {
        std::stable_sort(PotentialContainers.begin(), PotentialContainers.end(), comp());
    }

private:
  int Mass; // Масса паллеты
  int Width;
  int Height;
  int Length;
  int CarryingCapacity; // Грузоподъемность паллеты
  int CurrentLoadingWeight; // Текущая загрузка паллеты, вес
  int CurrentBlankVolume; // Текущий доступный объем
  std::vector<TContainer*> LoadedSpace;// Вектор Загруженных Контейнеров
  std::vector<TBorders> PotentialContainers;//Вектор Потенциальных Контейнеров
};

int main (int argc, char* argv[])
{
  TCargoSpace trailer(10, 10, 10, 15, 15);

  std::vector<TCargoGroup> inputStream;
  TCargoGroup cg;
  cg.Mass = 1;
  cg.Width = 3;
  cg.Length = 9;
  cg.Height = 1;
  cg.GroupId = 0;
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));
  cg.Length = 15;
  inputStream.push_back(std::move(cg));
  cg.Length = 1;
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));
  cg.Width = 1;
  cg.Height = 3;
  cg.Length = 7;
  inputStream.push_back(std::move(cg));
  cg.Width = 2;
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));
  inputStream.push_back(std::move(cg));


  trailer.LoadContainers(inputStream);


    std::ifstream ifs { R"(test.json)" };
    if ( !ifs.is_open() )
    {
        std::cerr << "Could not open file for reading\n";
        return EXIT_FAILURE;
    }

    //Перевод std::ifstream ifs на поток rapidjson isw
    rapidjson::IStreamWrapper isw { ifs };

    //Парсим наш json файл из входного потока
    rapidjson::Document test_document {};
    test_document.ParseStream( isw );

    //Выделяем буфер для записи нашего файла и генерируем текст
    rapidjson::StringBuffer buffer {};
    rapidjson::Writer<rapidjson::StringBuffer> writer { buffer };

    //Передаем поле "cargo_space"
    test_document[ "cargo_space" ].Accept( writer );

    if ( test_document.HasParseError() )
    {
        std::cout << "Error  : " <<  test_document.GetParseError()  << '\n'
                  << "Offset : " <<  test_document.GetErrorOffset() << '\n';
        return EXIT_FAILURE;
    }

    //Извлекаем строку
    const std::string jsonStr { buffer.GetString() };

    std::cout << jsonStr << '\n';

    system("pause");





    return 0;
}