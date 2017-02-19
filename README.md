# KiwiScheduler

[![Travis](https://img.shields.io/travis/CICM/HoaLibrary-Light.svg?label=travis)](https://travis-ci.org/CICM/HoaLibrary-Light) [![Appveyor](https://img.shields.io/appveyor/ci/pierreguillot/HoaLibrary-Light.svg?label=appveyor)](https://ci.appveyor.com/project/pierreguillot/HoaLibrary-Light/history)

[![Coverage Status](https://coveralls.io/repos/github/CICM/HoaLibrary-Light/badge.svg?branch=dev%2Fv2.3)](https://coveralls.io/github/CICM/HoaLibrary-Light?branch=dev%2Fv2.3) [![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://musicoll.github.io/KiwiScheduler/)

The scheduler manages time events, called tasks, in a multi-thread context. This implementation allows to have one tasks consumer with several tasks' producer.  Each producer, usually associated with one thread, owns a unique id that is used at the creation of tasks and that specify the producer to which they will be associated to.
