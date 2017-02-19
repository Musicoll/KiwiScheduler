# KiwiScheduler

[![Travis](https://img.shields.io/travis/Musicoll/KiwiScheduler.svg?label=travis)](https://travis-ci.org/Musicoll/KiwiScheduler) [![Appveyor](https://img.shields.io/appveyor/ci/pierreguillot/KiwiScheduler.svg?label=appveyor)](https://ci.appveyor.com/project/pierreguillot/kiwischeduler/history) [![Coverage Status](https://coveralls.io/repos/github/Musicoll/KiwiScheduler/badge.svg)](https://coveralls.io/github/Musicoll/KiwiScheduler) [![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://musicoll.github.io/KiwiScheduler/)

The scheduler manages time events, called tasks, in a multi-thread context. This implementation allows to have one tasks consumer with several tasks' producer.  Each producer, usually associated with one thread, owns a unique id that is used at the creation of tasks and that specify the producer to which they will be associated to.
