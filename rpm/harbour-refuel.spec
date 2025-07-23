Name:       harbour-refuel

Summary:    Refuel
Version:    0.1
Release:    1
License:    GPLv3
URL:        https://github.com/R1tschY/harbour-refuel
Source0:    %{name}-%{version}.tar.bz2
Source1:    .
Requires:   sailfishsilica-qt5 >= 0.10.9
BuildRequires:  pkgconfig(sailfishapp) >= 1.0.2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Network)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Positioning)
BuildRequires:  pkgconfig(Qt5Sql)
BuildRequires:  pkgconfig(Qt5Location)
BuildRequires:  pkgconfig(Qt5QuickTest)
BuildRequires:  desktop-file-utils
BuildRequires:  cmake >= 3.14

Requires: qt5-plugin-geoservices-osm
Requires: qt5-qtdeclarative-import-location
Requires: qt5-qtdeclarative-import-positioning

%description
Search for fuel prices in Germany


%prep
%setup -q -n %{name}-%{version}

%build

source %{SOURCE1}/../.env

cmake \
  -DCPM_SOURCE_CACHE=$PWD/.cpm-cache \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DPACKAGE_VERSION="%{version}" \
  -DPACKAGE_RELEASE="%{release}" \
  -DTANKERKOENIG_APIKEY=$TANKERKOENIG_APIKEY
cmake --build . -- %{?_smp_mflags}

%install
DESTDIR=%{buildroot} cmake --build . --target install


desktop-file-install --delete-original \
    --dir %{buildroot}%{_datadir}/applications \
    %{buildroot}%{_datadir}/applications/*.desktop

%files
%defattr(-,root,root,-)
%{_bindir}/%{name}
%{_datadir}/%{name}
%{_datadir}/applications/%{name}.desktop
%{_datadir}/icons/hicolor/*/apps/%{name}.png
