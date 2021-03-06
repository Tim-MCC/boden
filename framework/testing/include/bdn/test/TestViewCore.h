#ifndef BDN_TEST_TestViewCore_H_
#define BDN_TEST_TestViewCore_H_

#include <bdn/View.h>
#include <bdn/Window.h>
#include <bdn/test.h>
#include <bdn/test/testCalcPreferredSize.h>
#include <bdn/IUiProvider.h>
#include <bdn/RequireNewAlloc.h>
#include <bdn/Button.h>
#include <bdn/ColumnView.h>
#include <bdn/Array.h>

namespace bdn
{
    namespace test
    {

        /** Helper for tests that verify IViewCore implementations.*/
        template <class ViewType> class TestViewCore : public RequireNewAlloc<Base, TestViewCore<ViewType>>
        {
          public:
            /** Performs the tests.*/
            virtual void runTests()
            {
                _window = newObj<WindowForTest>(&getUiProvider());

                _window->setVisible(true);

                setView(createView());

                // sanity check: the view should not have a parent yet
                REQUIRE(_view->getParentView() == nullptr);

                SECTION("init")
                {
                    if (_view == cast<View>(_window)) {
                        // the view is a window. These always have a core from
                        // the start, so we cannot do any init tests with them.

                        // only check that the view core is indeed already
                        // there.
                        REQUIRE(_view->getViewCore() != nullptr);
                    } else {
                        // non-windows should not have a view core in the
                        // beginning (before they are added to the window).

                        REQUIRE(_view->getViewCore() == nullptr);

                        // run the init tests for them
                        runInitTests();
                    }
                }

                SECTION("postInit")
                {
                    initCore();

                    // view should always be visible for these tests
                    _view->setVisible(true);

                    // ensure that all pending initializations have finished.
                    P<TestViewCore<ViewType>> self = this;

                    CONTINUE_SECTION_WHEN_IDLE(self) { self->runPostInitTests(); };
                }
            }

          protected:
            /** Returns true if the view position can be manually changed.
                Returns false if this is a UI element whose position is
               controlled by an external entity.

                The default implementation returns true
                */
            virtual bool canManuallyChangePosition() const { return true; }

            /** Returns true if the view sizecan be manually changed.
                Returns false if this is a UI element whose size is controlled
                by an external entity.

                The default implementation returns true
                */
            virtual bool canManuallyChangeSize() const { return true; }

            /** Runs the tests that verify that the core initializes itself with
               the current property values of the outer view.

                The core is not yet initialized when this function is called

                The tests each modify an outer view property, then cause the
               core to be created (by calling initCore()) and then verify that
               the core has initialized itself correctly.
                */
            virtual void runInitTests()
            {
                SECTION("visible")
                {
                    _view->setVisible(true);

                    initCore();
                    verifyCoreVisibility();
                }

                SECTION("invisible")
                {
                    _view->setVisible(false);

                    initCore();
                    verifyCoreVisibility();
                }

                SECTION("padding")
                {
                    SECTION("default")
                    {
                        // the default padding of the outer view should be null
                        // (i.e. "use default").
                        REQUIRE(_view->padding().isNull());

                        initCore();
                        verifyCorePadding();
                    }

                    SECTION("explicit")
                    {
                        _view->setPadding(
                            UiMargin(UiLength::sem(11), UiLength::sem(22), UiLength::sem(33), UiLength::sem(44)));

                        initCore();
                        verifyCorePadding();
                    }
                }

                SECTION("bounds")
                {
                    _view->adjustAndSetBounds(Rect(110, 220, 660, 510));

                    initCore();
                    verifyInitialDummyCoreSize();
                }
            }

            /** Runs the tests that verify the core behaviour for operations
               that happen after the core is initialized.

                The core is already created/initialized when this is function is
               called.
                */
            virtual void runPostInitTests()
            {
                P<TestViewCore<ViewType>> self = this;

                SECTION("uiLengthToDips")
                {
                    REQUIRE(_core->uiLengthToDips(UiLength::none()) == 0);
                    REQUIRE(_core->uiLengthToDips(UiLength(17, UiLength::Unit::none)) == 0);
                    REQUIRE(_core->uiLengthToDips(UiLength::dip(0)) == 0);
                    REQUIRE(_core->uiLengthToDips(UiLength::sem(0)) == 0);
                    REQUIRE(_core->uiLengthToDips(UiLength::em(0)) == 0);

                    REQUIRE(_core->uiLengthToDips(UiLength::dip(17.34)) == 17.34);

                    double semSize = _core->uiLengthToDips(UiLength::sem(1));
                    REQUIRE(semSize > 0);
                    REQUIRE_ALMOST_EQUAL(_core->uiLengthToDips(UiLength::sem(3)), semSize * 3, 3);

                    double emSize = _core->uiLengthToDips(UiLength::em(1));
                    REQUIRE(emSize > 0);
                    REQUIRE_ALMOST_EQUAL(_core->uiLengthToDips(UiLength::em(3)), emSize * 3, 3);
                }

                SECTION("uiMarginToDipMargin")
                {
                    SECTION("none")
                    {
                        REQUIRE(_core->uiMarginToDipMargin(
                                    UiMargin(UiLength(10, UiLength::Unit::none), UiLength(20, UiLength::Unit::none),
                                             UiLength(30, UiLength::Unit::none), UiLength(40, UiLength::Unit::none))) ==
                                Margin(0, 0, 0, 0));
                    }

                    SECTION("dip")
                    {
                        REQUIRE(_core->uiMarginToDipMargin(UiMargin(10, 20, 30, 40)) == Margin(10, 20, 30, 40));
                    }

                    SECTION("sem")
                    {
                        double semDips = _core->uiLengthToDips(UiLength::sem(1));

                        Margin m = _core->uiMarginToDipMargin(
                            UiMargin(UiLength::sem(10), UiLength::sem(20), UiLength::sem(30), UiLength::sem(40)));
                        REQUIRE_ALMOST_EQUAL(m.top, 10 * semDips, 10);
                        REQUIRE_ALMOST_EQUAL(m.right, 20 * semDips, 20);
                        REQUIRE_ALMOST_EQUAL(m.bottom, 30 * semDips, 30);
                        REQUIRE_ALMOST_EQUAL(m.left, 40 * semDips, 40);
                    }

                    SECTION("em")
                    {
                        double emDips = _core->uiLengthToDips(UiLength::em(1));

                        Margin m = _core->uiMarginToDipMargin(
                            UiMargin(UiLength::em(10), UiLength::em(20), UiLength::em(30), UiLength::em(40)));
                        REQUIRE_ALMOST_EQUAL(m.top, 10 * emDips, 10);
                        REQUIRE_ALMOST_EQUAL(m.right, 20 * emDips, 20);
                        REQUIRE_ALMOST_EQUAL(m.bottom, 30 * emDips, 30);
                        REQUIRE_ALMOST_EQUAL(m.left, 40 * emDips, 40);
                    }
                }

                if (coreCanCalculatePreferredSize()) {
                    SECTION("calcPreferredSize")
                    bdn::test::_testCalcPreferredSize<ViewType, IViewCore>(_view, _core, this);
                }

                SECTION("visibility")
                {
                    SECTION("visible")
                    {
                        _view->setVisible(true);

                        CONTINUE_SECTION_WHEN_IDLE(self) { self->verifyCoreVisibility(); };
                    }

                    SECTION("invisible")
                    {
                        _view->setVisible(false);

                        CONTINUE_SECTION_WHEN_IDLE(self) { self->verifyCoreVisibility(); };
                    }

                    if (coreCanCalculatePreferredSize()) {
                        SECTION("noEffectOnPreferredSize")
                        {
                            // verify that visibility has no effect on the
                            // preferred size
                            Size prefSizeBefore = _core->calcPreferredSize();

                            _view->setVisible(true);

                            REQUIRE(self->_core->calcPreferredSize() == prefSizeBefore);

                            self->_view->setVisible(false);

                            REQUIRE(self->_core->calcPreferredSize() == prefSizeBefore);

                            self->_view->setVisible(true);

                            REQUIRE(self->_core->calcPreferredSize() == prefSizeBefore);
                        }
                    }
                }

                SECTION("padding")
                {
                    SECTION("custom")
                    {
                        _view->setPadding(UiMargin(11, 22, 33, 44));

                        CONTINUE_SECTION_WHEN_IDLE(self) { self->verifyCorePadding(); };
                    }

                    SECTION("default after custom")
                    {
                        // set a non-default padding, then go back to default
                        // padding.
                        _view->setPadding(UiMargin(11, 22, 33, 44));
                        _view->setPadding(nullptr);

                        CONTINUE_SECTION_WHEN_IDLE(self) { self->verifyCorePadding(); };
                    }

                    if (coreCanCalculatePreferredSize()) {
                        SECTION("effectsOnPreferredSize")
                        {
                            // For some UI elements on some platforms there may
                            // be a silent minimum padding. If we specify a
                            // smaller padding then the minimum will be used
                            // instead.

                            // So to verify the effects of padding we first set
                            // a big padding that we are fairly confident to be
                            // over any minimum.

                            UiMargin paddingBefore(UiLength::sem(10));

                            _view->setPadding(paddingBefore);

                            // wait a little so that sizing info is updated.
                            CONTINUE_SECTION_WHEN_IDLE(self, paddingBefore)
                            {
                                Size prefSizeBefore = self->_core->calcPreferredSize();

                                UiMargin additionalPadding(UiLength::sem(1), UiLength::sem(2), UiLength::sem(3),
                                                           UiLength::sem(4));
                                UiMargin increasedPadding =
                                    UiMargin(UiLength::sem(paddingBefore.top.value + additionalPadding.top.value),
                                             UiLength::sem(paddingBefore.right.value + additionalPadding.right.value),
                                             UiLength::sem(paddingBefore.bottom.value + additionalPadding.bottom.value),
                                             UiLength::sem(paddingBefore.left.value + additionalPadding.left.value));

                                // setting padding should increase the preferred
                                // size of the core.
                                self->_view->setPadding(increasedPadding);

                                CONTINUE_SECTION_WHEN_IDLE(self, prefSizeBefore, additionalPadding)
                                {
                                    // the padding should increase the preferred
                                    // size.
                                    Size prefSize = self->_core->calcPreferredSize();

                                    Margin additionalPaddingPixels =
                                        self->_view->uiMarginToDipMargin(additionalPadding);

                                    REQUIRE_ALMOST_EQUAL(prefSize, prefSizeBefore + additionalPaddingPixels,
                                                         Size(1, 1));
                                };
                            };
                        }
                    }
                }

                SECTION("adjustAndSetBounds")
                {
                    Rect bounds;
                    Point initialPosition = _view->position();

                    SECTION("no need to adjust")
                    {
                        // pre-adjust bounds so that we know that they are valid
                        bounds = Rect(110, 220, 660, 510);
                        bounds = _core->adjustBounds(bounds, RoundType::nearest, RoundType::nearest);
                    }

                    SECTION("need adjustments")
                    bounds = Rect(110.12345, 220.12345, 660.12345, 510.12345);

                    Rect returnedBounds = _view->adjustAndSetBounds(bounds);

                    // on some platform and with some view types (Linux / GTK
                    // with top level window) waiting for idle is not enough to
                    // ensure that the position actually changed. So instead we
                    // first wait for idle and then wait some additional seconds
                    // to ensure that our changes have been applied
                    CONTINUE_SECTION_WHEN_IDLE(self, bounds, returnedBounds)
                    {
                        CONTINUE_SECTION_AFTER_RUN_SECONDS(0.5, self, bounds, returnedBounds)
                        {
                            // the core size and position should always
                            // represent what is configured in the view.
                            self->verifyCorePosition();
                            self->verifyCoreSize();

                            if (!self->canManuallyChangePosition()) {
                                // when the view cannot modify its position then
                                // trying to set another position should yield
                                // the same resulting position
                                Rect returnedBounds2 = self->_core->adjustAndSetBounds(
                                    Rect(bounds.x * 2, bounds.y * 2, bounds.width, bounds.height));
                                REQUIRE(returnedBounds2 == returnedBounds);

                                CONTINUE_SECTION_WHEN_IDLE(self)
                                {
                                    // the core size and position should always
                                    // represent what is configured in the view.
                                    self->verifyCorePosition();
                                    self->verifyCoreSize();
                                };
                            }
                        };
                    };
                }

                if (coreCanCalculatePreferredSize()) {
                    SECTION("adjustAndSetBounds no effect on preferred size")
                    {
                        Size prefSizeBefore = _core->calcPreferredSize();

                        _view->adjustAndSetBounds(Rect(110, 220, 660, 510));

                        REQUIRE(self->_core->calcPreferredSize() == prefSizeBefore);
                    }
                }

                SECTION("adjustBounds")
                {
                    SECTION("no need to adjust")
                    {
                        Rect bounds(110, 220, 660, 510);

                        // pre-adjust the bounds
                        bounds = _core->adjustBounds(bounds, RoundType::nearest, RoundType::nearest);

                        List<RoundType> roundTypes{RoundType::nearest, RoundType::up, RoundType::down};

                        for (RoundType positionRoundType : roundTypes) {
                            for (RoundType sizeRoundType : roundTypes) {
                                SECTION("positionRoundType: " + std::to_string((int)positionRoundType) + ", " +
                                        std::to_string((int)sizeRoundType))
                                {
                                    Rect adjustedBounds = _core->adjustBounds(bounds, positionRoundType, sizeRoundType);

                                    // no adjustments are necessary. So we
                                    // should always get out the same that we
                                    // put in
                                    REQUIRE(adjustedBounds == bounds);
                                }
                            }
                        }
                    }

                    SECTION("need adjustments")
                    {

                        Rect bounds(110.12345, 220.12345, 660.12345, 510.12345);

                        List<RoundType> roundTypes{RoundType::down, RoundType::nearest, RoundType::up};

                        Array<Rect> adjustedBoundsArray;

                        for (RoundType sizeRoundType : roundTypes) {
                            for (RoundType positionRoundType : roundTypes) {
                                Rect adjustedBounds = _core->adjustBounds(bounds, positionRoundType, sizeRoundType);

                                adjustedBoundsArray.push_back(adjustedBounds);
                            }
                        }

                        // there is not much we can do to "verify" the bounds.
                        // However, there are some relationships of the rounding
                        // operations that must be true.

                        // down-rounded must be <= nearest <=up

                        for (int sizeRoundTypeIndex = 0; sizeRoundTypeIndex < 3; sizeRoundTypeIndex++) {
                            Point downRoundedPosition = adjustedBoundsArray[sizeRoundTypeIndex * 3 + 0].getPosition();
                            Point nearestRoundedPosition =
                                adjustedBoundsArray[sizeRoundTypeIndex * 3 + 1].getPosition();
                            Point upRoundedPosition = adjustedBoundsArray[sizeRoundTypeIndex * 3 + 2].getPosition();

                            REQUIRE(downRoundedPosition <= nearestRoundedPosition);
                            REQUIRE(nearestRoundedPosition <= upRoundedPosition);

                            // if canManuallyChangePosition returns false then
                            // it means that the view has no control over its
                            // position. That means that adjustBounds will
                            // always return the view's current position.
                            if (canManuallyChangePosition()) {
                                REQUIRE(downRoundedPosition <= bounds.getPosition());
                                REQUIRE(upRoundedPosition >= bounds.getPosition());
                            }
                        }

                        for (int positionRoundTypeIndex = 0; positionRoundTypeIndex < 3; positionRoundTypeIndex++) {
                            Size downRoundedSize = adjustedBoundsArray[0 * 3 + positionRoundTypeIndex].getSize();
                            Size nearestRoundedSize = adjustedBoundsArray[1 * 3 + positionRoundTypeIndex].getSize();
                            Size upRoundedSize = adjustedBoundsArray[2 * 3 + positionRoundTypeIndex].getSize();

                            REQUIRE(downRoundedSize <= nearestRoundedSize);
                            REQUIRE(nearestRoundedSize <= upRoundedSize);

                            // if canManuallyChangeSize returns false then it
                            // means that the view has no control over its size.
                            // That means that adjustBounds will always return
                            // the view's current size.
                            if (canManuallyChangeSize()) {
                                REQUIRE(downRoundedSize <= bounds.getSize());
                                REQUIRE(upRoundedSize >= bounds.getSize());
                            }
                        }
                    }
                }

                SECTION("invalidateSizingInfo invalidates parent sizing info")
                {
                    int invalidateCountBefore = _window->getInvalidateSizingInfoCount();

                    _view->invalidateSizingInfo(View::InvalidateReason::customDataChanged);

                    REQUIRE(_window->getInvalidateSizingInfoCount() > invalidateCountBefore);
                }
            }

            /** Causes the core object to be created. This is done by adding the
               view as a child to a visible view container or window.*/
            virtual void initCore()
            {
                if (_view != cast<View>(_window)) {
                    // the view might need control over its size to be able to
                    // do some of its test. Because of this we cannot add it to
                    // the window directly. Instead we add an intermediate
                    // ColumnView.
                    P<ColumnView> container = newObj<ColumnView>();
                    _window->setContentView(container);

                    container->addChildView(_view);
                }

                _core = _view->getViewCore();

                REQUIRE(_core != nullptr);
            }

            /** Verifies that the core's visible property matches that of the
             * outer view.*/
            virtual void verifyCoreVisibility() = 0;

            /** Verifies that the core's padding property matches that of the
             * outer view.*/
            virtual void verifyCorePadding() = 0;

            /** Verifies that the core's size property has the initial dummy
               value used directly after initialization.*/
            virtual void verifyInitialDummyCoreSize() = 0;

            /** Verifies that the core's position property matches that of the
             * outer view.*/
            virtual void verifyCorePosition() = 0;

            /** Verifies that the core's size property matches that of the outer
             * view.*/
            virtual void verifyCoreSize() = 0;

            /** Returns the UiProvider to use.*/
            virtual IUiProvider &getUiProvider() = 0;

            /** Creates the view object to use for the tests.*/
            virtual P<View> createView() = 0;

            /** Sets the view object to use for the tests.*/
            virtual void setView(View *view) { _view = view; }

            /** Returns true if the view core can calculate its preferred size.
                Some core types depend on the outer view to calculate the
               preferred size instead.

                The default implementation returns true.
                */
            virtual bool coreCanCalculatePreferredSize() { return true; }

          protected:
            class WindowForTest : public Window
            {
              public:
                WindowForTest(IUiProvider *uiProvider = nullptr) : Window(uiProvider) {}

                void invalidateSizingInfo(InvalidateReason reason) override
                {
                    _invalidateSizingInfoCount++;

                    Window::invalidateSizingInfo(reason);
                }

                int getInvalidateSizingInfoCount() const { return _invalidateSizingInfoCount; }

              private:
                int _invalidateSizingInfoCount = 0;
            };

            P<WindowForTest> _window;
            P<View> _view;

            P<IViewCore> _core;
        };
    }
}

#endif
