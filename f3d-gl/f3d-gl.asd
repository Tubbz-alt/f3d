;;;; -*- Mode: LISP; Syntax: ANSI-Common-Lisp; Base: 10 -*-

(in-package #:cl-user)

;;;
;;; Add features that will control the UI substrate.  The user has the
;;; option of adding :cocoa to *features* to induce MacOSX-native UI
;;; windows and widgets, as opposed to X11.

;;; Core will not need these:

;;; (pushnew :tkgui *features*)

;;; #+win32
;;; (pushnew :wgl *features*)

#-(or cocoa win32)
(pushnew :glx *features*)

;; (sb-posix::setenv "RADIUS" "/opt/IU/radius")

;;; This is FREEDIUS modified to be usable with the quicklisp / asdf
;;; framework.  The ultimate goals are (a) to modularize functionality
;;; so that the base stuff (images, geometry, transforms, objects) can
;;; be loaded easily within quicklisp, and (b) to become more
;;; compatible with the Lisp community, which is moving toward
;;; standards for FFIs, network-based system definition, retrieval,
;;; etc.
;;;
;;; The UI layer should retain most of its concepts, but we should be
;;; able to exploit other Lisp GUI efforts.  I continue to be troubled
;;; by the maze of OS dependencies induced by Tcl/Tk, not to mention
;;; the forced presence of a Tcl interpreter with its own opaque
;;; interpreter stack.

;;; The cl-cme-tk system should represent a "reference" that works
;;; just like the original FREEDIUS, possibly restricted to X windows.
;;; It has no dependencies on other systems.

;;; This system uses ASDF extensions that are provided by a companion
;;; system called :cl-cmake, that allows us to build and load shared
;;; objects.  This is still unsatisfying, since there are other
;;; dependencies lurking in those systems that will probably throw
;;; people off.  In particular, this system depends on jpeg, OpenGL,
;;; and Tcl/Tk, and requires the installation of header files in
;;; "nice" locations.


;;; Reader macros are used here to designate modules that can be split
;;; off into other systems:
;;;
;;; f3d-core (no macro)
;;; f3d-gl   (OpenGL API)
;;; f3d-geom (object geometry)
;;; f3d-tk   (Tcl/Tk-specific code)


(asdf:defsystem :f3d-gl
  :name "f3d-gl"
  :defsystem-depends-on ("cl-cmake")
  :depends-on (:f3d-core)   ;; does it?
  :maintainer "Chris Connolly, SRI International <connolly@ai.sri.com>"
  :license "Mozilla"
  :description "FREEDIUS 2D and 3D object representation."
  :components 
  (
   (:module :gl
            :serial t
            :components
            ((:shared-object "libf3dglffi"
			     :properties
                             ;; for window system, but we should generalize:
			     ((:cmake-options
			       . (#+glx   "-DFREEDIUS_USE_X11=1"
				  #+cocoa "-DFREEDIUS_USE_COCOA=1"
				  ))))))
   (:module :gl-ffi
            :depends-on (:gl)
	    :serial t
	    :components
	    ((:file "gl-pkg")
             (:file "gl-macros")
	     (:file "gl-ffi")
	     (:file "glext-ffi")
	     (:file "glu-ffi")
	     (:file "gl-import")
	     (:file "gl-utils")
             (:file "glut-fonts")
             (:file "glut-defs")
	     (:file "gl-pkg-export-all")
	     ))

   ;; Moved from lisptk.  There should be no Tk dependences here.  We
   ;; should also streamline this API - these are OpenGL "helper"
   ;; functions for various window systems and could be improved:
   (:module :tk-gl
	    :serial t
	    :components
	    (
	      #+wgl (:file  "wgl-ffi")
              #+agl (:file  "agl-ffi")
	      #+cocoa  (:file "cocoa-ffi")
	      #+glx    (:file "glx-ffi")
	      #+glx    (:file "x11-error-handler")
	     ))

   ))
