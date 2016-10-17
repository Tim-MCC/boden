#ifndef BDN_WEB_WindowCore_H_
#define BDN_WEB_WindowCore_H_

#include <bdn/IWindowCore.h>
#include <bdn/Window.h>

#include <bdn/web/ViewCore.h>


namespace bdn
{
namespace web
{

class WindowCore : public ViewCore, BDN_IMPLEMENTS IWindowCore
{
public:
    WindowCore( Window* pOuterWindow )
        : ViewCore(pOuterWindow, "div")     // a "window" is simply a top level div
    {
        setTitle( pOuterWindow->title() );

        // the window div always has the same size as the browser window.        
        _domObject["style"].set("width", "100%");
        _domObject["style"].set("height", "100%");

        emscripten_set_resize_callback( "#window", static_cast<void*>(this), false, &WindowCore::_resizedCallback);

        // update the outer view's bounds with the current bounds from this window.
        // Do this in an async call, because the current callstack is unknown and the
        // property changes might otherwise have unexpected side effects.
        P<WindowCore> pThis = this;
        asyncCallFromMainThread(
            [pThis]()
            {
                pThis->updateOuterViewBounds();                
            });
    }

    ~WindowCore()
    {
        if(!_domObject.isNull() && !_domObject.isUndefined())
        {
            // delete the DOM object
            emscripten::val parent = _domObject["parentNode"];
            if(!parent.isNull())
                parent.call<void>("removeChild", _domObject);
        }
    }



    Rect adjustAndSetBounds(const Rect& bounds) override
    {
        // we cannot modify our size and position. Do nothing and return our current bounds.
        return _getBounds();
    }


    Rect adjustBounds(const Rect& requestedBounds, RoundType positionRoundType, RoundType sizeRoundType) const override
    {
        // we cannot modify our size and position. Do nothing and return our current bounds.
        return _getBounds();
    }
    


    void setTitle(const String& title) override
    {
        // set the DOM document title
        emscripten::val docVal = emscripten::val::global("document");

        docVal.set("title", title.asUtf8() );
    }


    Rect getContentArea() override
    {
        P<View> pView = getOuterViewIfStillAttached();
        if(pView!=nullptr)
            return Rect( Point(0,0), pView->size().get() );
        else
            return Rect();
    }


    Size calcWindowSizeFromContentAreaSize(const Size& contentSize) override
    {
        // our "window" is simply the size of the div. It as no borders.
        // So "window" size = content size.
        return contentSize;
    }


    Size calcContentAreaSizeFromWindowSize(const Size& windowSize) override
    {
        // our "window" is simply the size of the div. It as no borders.
        // So "window" size = content size.
        return windowSize;
    }


    Size calcMinimumSize() const override
    {
        // don't have a minimum size since the title bar is not connected to our window div.
        // (the title is displayed in the browser tab bar).
        return Size(0, 0);
    }
    

    Rect getScreenWorkArea() const override
    {
        emscripten::val windowVal = emscripten::val::global("window");

        int width = windowVal["innerWidth"].as<int>();
        int height = windowVal["innerHeight"].as<int>();

        return Rect(0, 0, width, height);
    }
    
private:
    Rect _getBounds() const
    {
        int width = _domObject["offsetWidth"].as<int>();
        int height = _domObject["offsetHeight"].as<int>();

        // we do not know the actual position of our window on the screen.
        // The web browser does not expose that.

        return Rect(0, 0, width, height);
    }

    void updateOuterViewBounds()
    {
        if(!_domObject.isNull() && !_domObject.isUndefined())
        {
            P<View> pView = getOuterViewIfStillAttached();
            if(pView!=nullptr)
                pView->adjustAndSetBounds( _getBounds() );
        }
    }


    bool resized(int eventType, const EmscriptenUiEvent* pEvent)
    {
        updateOuterViewBounds();

        return false;
    }

    static EM_BOOL _resizedCallback(int eventType, const EmscriptenUiEvent* pEvent, void* pUserData)
    {
        return static_cast<WindowCore*>(pUserData)->resized(eventType, pEvent);
    }

};


}
}

#endif

