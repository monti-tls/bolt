## bolt

Work in progress...

### Naming conventions

#### C-like sources (mainly VM code)

```C++
struct lowercase_with_underscores
{};

void function_name(...)
{}

enum
{
    UPPER_CASE_UNDERSCORED
}

#### C++ OO sources (other code : parsing, user interface)

class CamelCase
{
public:
    int publicMethod();
    
private:
    int M_privateMethod();

public:
    int publicMember;
    
private:
    int m_privateMember;
};
