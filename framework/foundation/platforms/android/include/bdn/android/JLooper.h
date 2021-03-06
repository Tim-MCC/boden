#pragma once

#include <bdn/java/JObject.h>

#include <bdn/android/JLooper.h>

namespace bdn
{
    namespace android
    {

        /** Accessor for Java android.os.Looper objects.*/
        class JLooper : public bdn::java::JObject
        {
          public:
            /** @param javaRef the reference to the Java object.
             *      The JObject instance will copy this reference and keep its
             * type. So if you want the JObject instance to hold a strong
             * reference then you need to call toStrong() on the reference first
             * and pass the result.
             *      */
            explicit JLooper(const bdn::java::Reference &javaRef) : JObject(javaRef) {}

            /** Returns the application's main looper, which lives in the main
             * thread of the application. */
            static JLooper getMainLooper()
            {
                static bdn::java::MethodId methodId;

                return invokeStatic_<JLooper>(getStaticClass_(), methodId, "getMainLooper");
            }

            /** Returns the JClass object for this class.
             *
             *  Note that the returned class object is not necessarily unique
             * for the whole process. You might get different objects if this
             * function is called from different shared libraries.
             *
             *  If you want to check for type equality then you should compare
             * the type name (see getTypeName() )
             *  */
            static bdn::java::JClass &getStaticClass_()
            {
                static bdn::java::JClass cls("android/os/Looper");

                return cls;
            }

            bdn::java::JClass &getClass_() override { return getStaticClass_(); }
        };
    }
}
