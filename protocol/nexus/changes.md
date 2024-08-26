# NeXus Definition Changes

The modifications according to the deliverable of WP2 are implemented in the ./protocol/nexus/definitions directory which is a clone of https://github.com/nexusformat/definitions.git. The modifications according to WP2 are sketched in ./protocol/nexus/2024_08_SECoP_NeXus_2.pptx or ./protocol/nexus/2024_08_SECoP_NeXus_2.pdf, respectively. Some modifications were discussed at https://github.com/jkotan/secop-file-examples/issues.

Clone the SECoP repository to a local source, switch to the definitions branch, and open the file definitions/build/manual/build/html/index.html in your browser, e.g. using the command 'firefox protocol/nexus/definitions/build/manual/build/html/index.html', to browse the modified NeXus documentation.

The following changes were applied:

- adding humidity, viscosity, and concentration to measurement field of NXsensor
- adding missing NXsensor/measurement values to NXsample; the following data fields were added to NXsample:
  - pH (adding NX_MOLAR_DENSITY to units, see nxdlTypes.xsd)
  - conductivity (adding NX_CONDUCTIVITY to units, see nxdlTypes.xsd)
  - resistance (adding NX_RESISTANCE to units, see nxdlTypes.xsd)
  - voltage
  - flow (adding NX_FLOW to units, see nxdlTypes.xsd)
  - strain
  - shear
  - surface_pressure
  - humidity
  - viscosity
- adding corresponding environment groups to NXsample:
  - electric_field_env
  - stress_field_env
  - pressure_env
  - pH_env
  - voltage_env
  - flow_env
  - strain_env
  - shear_env
  - surface_pressure_env
  - humidity_env
  - concentration_env
  - Since I am not sure if these properties are controlled by environments, the following environment groups were NOT added:
    - conductivity
    - resistance
    - viscosity
- adding (arbitrary) ENVIRONMENT groups to NXsample 
- modifying temperature field in NXsample according to similar fields (e.g. magnetic_field):
  - changing rank from 'anyRank' to 1
  - removing 'This could be a scanned variable'

Two more general things:
- There was a discussion about adding a separate NXsample group for sample environment (which I don't like so much because 2 NXsample groups could introduce some ambiguity). However, the changes made to NXsensor and NXsample are required independently of realisation.
- In a recent discussion with Peter possible problems become apparent when arranging SE (meta)data automatically (e.g. by script) since HDF could not handle arrays of mixed types (e.g. strings and floats). As a result, complex data types could cause errors in such a script. It could be required that SECnodes deliver additionally a recommendation about how its data should be stored/arranged.
