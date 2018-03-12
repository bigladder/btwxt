#include<iostream>


class AddThisInt{
public:
  AddThisInt(int x);
  int x;
  int operator()(int y)
  {
      return x + y;
  }
};

AddThisInt::AddThisInt(int x):
x(x)
{};


int main()
{
  int a = 9;
  AddThisInt::AddThisInt add_nine(a);
  int b = 7;
  int result = add_nine(b);
  std::cout << a << " + " << b << " = " << result << std::endl;


  a = 12;
  AddThisInt::AddThisInt add_twelve(a);
  b = -2;
  result = add_twelve(b);
  std::cout << a << " + " << b << " = " << result << std::endl;


  return 0;
};
