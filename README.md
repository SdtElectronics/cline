# cline
**cline** - the C++ interpreter online

Public service hosted at **[cppcli.net](cppcli.net)**

## Status
Public service is under testing.

The procedure to build and deploy this service is involved. We are working on the containerization of this project to facilitate self-hosting.

## Get involved
Submit an issue for feature request / bug report.

Pull requests are welcomed.

Please note that interpreter-internal issues are not handled here. They should be submitted to the [CERN Root](https://root.cern/) team / the [cling](https://root.cern/cling/) project.

## Credits
* Backend is wrapped around the C++ interpreter [cling](https://root.cern/cling/) .
* Web service is powered by a [modified version](https://github.com/SdtElectronics/websocketd-AF_UNIX) of [websocketd](https://github.com/joewalnes/websocketd).

## License
All source in [cline](./cline) is under the [LGPL](https://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html) license unless otherwise stated.

All source in [www](./www) is under the [BSD 3-Clause](https://opensource.org/licenses/BSD-3-Clause) license unless otherwise stated.
