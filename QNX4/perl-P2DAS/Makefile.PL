use 5.008002;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'P2DAS',
    VERSION_FROM      => 'lib/P2DAS.pm', # finds $VERSION
    PREREQ_PM         => {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/P2DAS.pm', # retrieve abstract from module
       AUTHOR         => 'Norton Allen <nort@harvard.edu>') : ()),
);
