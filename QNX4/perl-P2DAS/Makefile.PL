use 5.006;
use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    NAME              => 'P2DAS',
    VERSION_FROM      => 'lib/P2DAS.pm',
    PREREQ_PM         => { QNX => 0 },
    ($] >= 5.005 ?     ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM  => 'lib/P2DAS.pm', # retrieve abstract from module
       AUTHOR         => 'Norton Allen <allen@huarp.harvard.edu>') : ()),
);
