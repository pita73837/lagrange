**Lagrange** is a [Python](http://www.python.org) package implementing likelihood models for geographic range evolution on phylogenetic trees, with methods for inferring rates of dispersal and local extinction and ancestral ranges.

This software implements methods described in [Ree, R H and S A Smith. 2008. Maximum likelihood inference of geographic range evolution by dispersal, local extinction, and cladogenesis. Systematic Biology 57(1):4-14.](http://www.informaworld.com/openurl?genre=article&issn=1063%2d5157&volume=57&issue=1&spage=4)

See also [Ree, R H, B R Moore, C O Webb, and M J Donoghue. 2005. A likelihood framework for inferring the evolution of geographic range on phylogenetic trees. Evolution 59(11):2299-2311](http://www.bioone.org/perlserv/?request=get-abstract&doi=10.1554%2F05-172.1&ct=1)

Analyses are run as Python scripts that use the Lagrange API. Most users will want to visit http://www.reelab.net/lagrange, and use the web-based tool to configure their analysis and generate a script that is downloaded and run locally.

Running analyses thus requires installing the latest snapshot release of Lagrange and its dependencies (see System Requirements below).

For help running Lagrange, contact [Rick Ree](mailto:rree_at_fieldmuseum_dot_org) or [Stephen Smith](http://www.blackrim.org).

Source code: https://github.com/rhr/lagrange-python

### System requirements ###
  * [Python](http://www.python.org) 2.7
  * [Scipy](http://www.scipy.org) - scientific libraries for python (and [numpy](http://numpy.scipy.org) - numerical libraries)
    * for Debian Linux and derivatives, e.g. Ubuntu:
> > > `sudo apt-get install python-scipy`
    * alternatively, use the [Anaconda](https://store.continuum.io/cshop/anaconda) Python distribution, which bundles all the requirements -- for Mac, Windows, and Linux

#### C++ update (July 16, 2010) ####
There is a BETA binary (executable) of the c++ version of lagrange (same method just different language for speed and some new analyses) in the downloads section. AFTER DOWNLOADING AND UNARCHIVING, run chmod +x lagrange\_cpp and it should work if you are running 10.6 MacOSX. There will eventually be BETA's for Linux (though source is easier for Linux users so see below) and MacOSX 10.5 soon. If you would like to compile from source get the source from github (the line is git://github.com/blackrim/lagrange.git) and get one of the BETA executables and follow the instructions in the manual. All the updated info and releases will be here just my development code is hosted on github -- so unless you are diving into the code, stay here for information on lagrange.