#ifndef BDN_UiMargin_H_
#define BDN_UiMargin_H_

#include <bdn/UiLength.h>

namespace bdn
{

    /** Represents the size of a margin or border around a UI element.

        In contast to #Margin, UiMargin specifies the margin sizes with UiLength
        objects, thus allowing use of more abstract sizes that depend on the
       screen and system settings.

        */
    struct UiMargin
    {
      public:
        UiMargin() {}

        UiMargin(const UiLength &all) : top(all), right(all), bottom(all), left(all) {}

        UiMargin(const UiLength &topBottom, const UiLength &leftRight)
            : top(topBottom), right(leftRight), bottom(topBottom), left(leftRight)
        {}

        UiMargin(const UiLength &top, const UiLength &right, const UiLength &bottom, const UiLength &left)
            : top(top), right(right), bottom(bottom), left(left)
        {}

        UiLength top;
        UiLength right;
        UiLength bottom;
        UiLength left;
    };

    template <typename CHAR_TYPE, class CHAR_TRAITS>
    std::basic_ostream<CHAR_TYPE, CHAR_TRAITS> &operator<<(std::basic_ostream<CHAR_TYPE, CHAR_TRAITS> &stream,
                                                           const UiMargin &m)
    {
        return stream << "(" << m.top << ", " << m.right << ", " << m.bottom << ", " << m.left << ")";
    }
}

inline bool operator==(const bdn::UiMargin &a, const bdn::UiMargin &b)
{
    return (a.top == b.top && a.right == b.right && a.bottom == b.bottom && a.left == b.left);
}

inline bool operator!=(const bdn::UiMargin &a, const bdn::UiMargin &b) { return !operator==(a, b); }

#endif
