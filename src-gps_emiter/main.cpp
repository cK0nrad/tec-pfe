/*
    Simple UDS server that simulate a GPS device
    allow for multiple client
*/

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <poll.h>
#include <memory>
#include <mutex>
#include <shared_mutex>

#define SOCKET_PATH "/tmp/tec_gps.socket"

#define FAKE_LENGTH 1083
// very, very bad practive but I don't want to make
// sqlite3 work on the emulator

const double POSITION[][2] = {
    {50.728208, 5.351142},
    {50.728206, 5.35115},
    {50.728177, 5.351219},
    {50.72795, 5.351805},
    {50.727842, 5.352071},
    {50.727672, 5.352486},
    {50.727757, 5.35258},
    {50.727819, 5.352665},
    {50.727894, 5.35276},
    {50.728015, 5.35291},
    {50.728341, 5.353305},
    {50.728664, 5.353681},
    {50.729041, 5.354038},
    {50.729426, 5.354366},
    {50.729554, 5.354495},
    {50.729614, 5.354554},
    {50.729708, 5.354693},
    {50.729561, 5.354819},
    {50.729282, 5.355002},
    {50.729105, 5.355117},
    {50.728847, 5.355291},
    {50.728715, 5.355415},
    {50.728627, 5.355508},
    {50.728606, 5.355531},
    {50.728606, 5.355531},
    {50.728489, 5.355658},
    {50.72843, 5.355739},
    {50.728396, 5.35582},
    {50.728364, 5.355885},
    {50.728329, 5.355979},
    {50.728357, 5.355999},
    {50.728441, 5.356103},
    {50.728488, 5.356202},
    {50.728542, 5.356342},
    {50.728577, 5.356501},
    {50.728639, 5.356708},
    {50.728657, 5.356771},
    {50.728657, 5.356771},
    {50.728854, 5.357429},
    {50.728914, 5.35761},
    {50.729011, 5.357876},
    {50.729118, 5.35818},
    {50.729174, 5.358354},
    {50.729221, 5.358515},
    {50.729413, 5.35906},
    {50.729562, 5.359565},
    {50.729596, 5.359681},
    {50.729662, 5.359888},
    {50.729662, 5.359888},
    {50.729694, 5.359989},
    {50.729827, 5.360404},
    {50.729862, 5.360766},
    {50.729866, 5.360846},
    {50.729864, 5.360914},
    {50.729862, 5.360973},
    {50.729857, 5.361068},
    {50.72985, 5.361195},
    {50.729844, 5.36129},
    {50.72984, 5.361365},
    {50.729834, 5.361473},
    {50.729832, 5.361638},
    {50.729832, 5.361651},
    {50.729837, 5.361787},
    {50.729844, 5.361862},
    {50.729872, 5.362162},
    {50.729876, 5.362196},
    {50.729898, 5.362357},
    {50.729917, 5.36249},
    {50.729996, 5.363048},
    {50.730079, 5.363569},
    {50.730128, 5.363827},
    {50.730155, 5.363963},
    {50.730228, 5.364276},
    {50.730392, 5.364911},
    {50.73043, 5.365061},
    {50.730596, 5.365813},
    {50.730648, 5.366014},
    {50.730685, 5.366133},
    {50.730705, 5.366194},
    {50.730766, 5.36636},
    {50.730818, 5.366481},
    {50.730872, 5.366591},
    {50.730876, 5.366601},
    {50.73098, 5.366779},
    {50.731027, 5.366855},
    {50.731054, 5.366897},
    {50.731054, 5.366897},
    {50.731287, 5.367265},
    {50.731368, 5.367392},
    {50.731457, 5.367537},
    {50.731542, 5.367676},
    {50.731556, 5.367699},
    {50.731577, 5.36774},
    {50.731639, 5.36786},
    {50.731654, 5.367889},
    {50.73174, 5.368084},
    {50.731755, 5.368119},
    {50.731812, 5.368264},
    {50.732601, 5.370427},
    {50.732941, 5.37135},
    {50.733098, 5.371774},
    {50.733217, 5.372116},
    {50.7333, 5.372333},
    {50.73335, 5.372483},
    {50.733388, 5.37261},
    {50.733425, 5.372757},
    {50.733456, 5.372913},
    {50.733485, 5.373039},
    {50.733504, 5.373147},
    {50.733519, 5.373267},
    {50.733529, 5.373398},
    {50.733536, 5.373511},
    {50.733538, 5.373581},
    {50.733538, 5.373581},
    {50.733539, 5.373645},
    {50.733543, 5.373857},
    {50.733534, 5.374101},
    {50.733502, 5.374495},
    {50.733475, 5.374675},
    {50.733446, 5.374861},
    {50.73341, 5.37505},
    {50.733361, 5.375266},
    {50.733222, 5.375679},
    {50.733123, 5.375924},
    {50.7331, 5.375991},
    {50.733072, 5.376071},
    {50.733037, 5.376195},
    {50.732992, 5.376404},
    {50.73297, 5.376507},
    {50.73294, 5.376669},
    {50.732896, 5.376896},
    {50.732883, 5.376965},
    {50.732862, 5.377088},
    {50.732834, 5.377255},
    {50.732825, 5.377328},
    {50.732813, 5.377431},
    {50.732811, 5.377502},
    {50.732809, 5.377566},
    {50.732816, 5.377682},
    {50.732826, 5.377765},
    {50.732846, 5.377876},
    {50.732929, 5.378266},
    {50.732929, 5.378266},
    {50.732962, 5.378422},
    {50.732982, 5.378517},
    {50.733025, 5.378719},
    {50.733053, 5.378872},
    {50.732764, 5.379129},
    {50.732654, 5.37923},
    {50.732584, 5.379288},
    {50.732377, 5.379454},
    {50.732319, 5.379488},
    {50.732283, 5.3795},
    {50.732256, 5.379499},
    {50.732234, 5.379489},
    {50.73221, 5.379473},
    {50.732151, 5.37941},
    {50.732069, 5.379312},
    {50.731954, 5.379164},
    {50.731875, 5.37907},
    {50.731819, 5.37901},
    {50.731773, 5.378969},
    {50.731726, 5.378932},
    {50.731666, 5.378904},
    {50.731627, 5.378898},
    {50.731612, 5.378895},
    {50.731587, 5.378895},
    {50.731532, 5.378897},
    {50.731511, 5.3789},
    {50.731321, 5.378932},
    {50.731234, 5.378947},
    {50.731155, 5.378964},
    {50.730996, 5.378996},
    {50.730916, 5.379013},
    {50.730887, 5.379018},
    {50.730757, 5.379045},
    {50.730733, 5.379053},
    {50.730704, 5.379062},
    {50.730655, 5.379084},
    {50.730612, 5.379108},
    {50.730577, 5.379136},
    {50.730519, 5.379201},
    {50.730514, 5.37921},
    {50.730455, 5.379305},
    {50.730442, 5.379336},
    {50.730385, 5.379475},
    {50.730322, 5.379653},
    {50.730293, 5.379733},
    {50.730283, 5.379761},
    {50.730273, 5.379791},
    {50.730208, 5.379921},
    {50.730101, 5.380123},
    {50.729964, 5.380344},
    {50.729737, 5.380648},
    {50.729656, 5.380751},
    {50.7296, 5.380796},
    {50.729537, 5.380847},
    {50.729449, 5.380921},
    {50.729137, 5.381129},
    {50.728878, 5.381289},
    {50.728732, 5.381368},
    {50.728422, 5.381584},
    {50.72826, 5.381743},
    {50.728068, 5.381948},
    {50.727678, 5.382448},
    {50.727307, 5.382941},
    {50.727025, 5.383329},
    {50.727019, 5.383339},
    {50.726184, 5.384561},
    {50.725947, 5.384916},
    {50.72559, 5.385476},
    {50.725529, 5.385567},
    {50.725409, 5.385688},
    {50.725372, 5.385722},
    {50.72508, 5.385929},
    {50.725051, 5.385948},
    {50.725051, 5.385948},
    {50.724893, 5.386052},
    {50.724768, 5.386161},
    {50.72468, 5.386263},
    {50.724499, 5.386519},
    {50.723989, 5.387354},
    {50.723688, 5.387904},
    {50.723422, 5.388486},
    {50.723422, 5.388486},
    {50.723419, 5.388493},
    {50.723107, 5.389179},
    {50.722896, 5.389633},
    {50.722727, 5.389991},
    {50.722653, 5.390169},
    {50.722557, 5.390422},
    {50.722392, 5.390828},
    {50.722392, 5.390828},
    {50.722341, 5.390953},
    {50.722182, 5.391346},
    {50.72207, 5.391502},
    {50.721977, 5.391622},
    {50.721889, 5.391734},
    {50.721779, 5.391901},
    {50.721733, 5.391992},
    {50.721642, 5.392165},
    {50.721292, 5.392972},
    {50.721055, 5.39342},
    {50.720915, 5.393729},
    {50.720822, 5.393942},
    {50.720761, 5.394034},
    {50.720672, 5.394172},
    {50.720585, 5.394253},
    {50.720498, 5.394327},
    {50.72041, 5.394413},
    {50.720224, 5.39458},
    {50.720224, 5.39458},
    {50.72021, 5.394593},
    {50.719988, 5.394762},
    {50.719635, 5.395106},
    {50.719231, 5.39545},
    {50.719216, 5.395463},
    {50.719058, 5.395621},
    {50.718824, 5.395952},
    {50.71866, 5.396196},
    {50.718437, 5.396492},
    {50.717974, 5.396996},
    {50.7178, 5.397221},
    {50.7178, 5.397221},
    {50.717518, 5.397587},
    {50.717492, 5.397631},
    {50.717464, 5.397622},
    {50.71744, 5.39763},
    {50.717425, 5.397644},
    {50.717413, 5.39766},
    {50.717405, 5.397681},
    {50.717399, 5.39771},
    {50.717403, 5.397745},
    {50.717407, 5.397766},
    {50.717416, 5.397784},
    {50.717423, 5.397798},
    {50.717345, 5.397912},
    {50.717221, 5.398094},
    {50.716805, 5.398594},
    {50.716678, 5.398753},
    {50.716244, 5.399464},
    {50.715301, 5.400806},
    {50.715196, 5.400943},
    {50.71486, 5.401425},
    {50.714669, 5.401679},
    {50.714669, 5.401679},
    {50.714482, 5.401925},
    {50.714223, 5.402289},
    {50.714134, 5.402434},
    {50.714028, 5.402612},
    {50.713919, 5.402804},
    {50.713821, 5.402953},
    {50.713708, 5.403177},
    {50.713288, 5.40426},
    {50.713085, 5.404841},
    {50.71299, 5.405124},
    {50.712908, 5.4055},
    {50.712865, 5.405737},
    {50.71283, 5.405902},
    {50.712753, 5.406141},
    {50.712593, 5.406555},
    {50.711788, 5.408378},
    {50.71166, 5.408696},
    {50.711452, 5.409219},
    {50.7109, 5.411069},
    {50.710228, 5.412919},
    {50.710019, 5.413366},
    {50.709874, 5.413659},
    {50.70986, 5.413682},
    {50.70971, 5.413971},
    {50.709498, 5.414351},
    {50.709498, 5.414351},
    {50.709352, 5.414613},
    {50.708713, 5.415772},
    {50.708623, 5.415935},
    {50.708541, 5.416059},
    {50.708458, 5.416149},
    {50.70808, 5.416226},
    {50.707823, 5.416238},
    {50.707011, 5.416312},
    {50.707011, 5.416312},
    {50.706863, 5.416326},
    {50.706478, 5.41638},
    {50.706462, 5.41649},
    {50.70626, 5.417182},
    {50.706107, 5.417787},
    {50.706088, 5.417868},
    {50.705995, 5.418337},
    {50.705965, 5.418491},
    {50.705905, 5.418947},
    {50.705783, 5.419979},
    {50.705641, 5.421091},
    {50.705618, 5.421258},
    {50.705596, 5.421419},
    {50.705575, 5.421547},
    {50.705545, 5.421702},
    {50.705247, 5.422994},
    {50.704983, 5.424058},
    {50.70494, 5.424246},
    {50.70489, 5.424447},
    {50.704779, 5.424933},
    {50.704736, 5.425122},
    {50.704672, 5.425377},
    {50.70454, 5.425947},
    {50.704426, 5.426434},
    {50.7043, 5.426978},
    {50.704224, 5.427252},
    {50.704156, 5.427489},
    {50.704079, 5.427759},
    {50.703994, 5.428033},
    {50.703908, 5.428304},
    {50.703772, 5.428761},
    {50.703677, 5.429064},
    {50.703548, 5.429458},
    {50.703376, 5.429969},
    {50.703239, 5.430368},
    {50.703162, 5.430575},
    {50.703107, 5.430719},
    {50.703046, 5.43087},
    {50.702951, 5.431076},
    {50.702861, 5.431267},
    {50.702783, 5.431425},
    {50.702716, 5.431576},
    {50.702665, 5.431702},
    {50.702622, 5.431844},
    {50.702596, 5.431973},
    {50.702577, 5.43211},
    {50.702564, 5.432225},
    {50.702531, 5.432686},
    {50.702497, 5.433267},
    {50.702481, 5.433619},
    {50.702476, 5.433701},
    {50.702471, 5.433796},
    {50.702467, 5.433897},
    {50.702464, 5.433979},
    {50.702462, 5.434058},
    {50.702464, 5.434167},
    {50.702471, 5.434292},
    {50.702436, 5.434326},
    {50.702404, 5.434369},
    {50.702368, 5.434438},
    {50.702322, 5.434561},
    {50.702297, 5.434667},
    {50.702292, 5.43469},
    {50.702292, 5.43469},
    {50.702273, 5.43479},
    {50.702248, 5.434924},
    {50.702224, 5.435102},
    {50.702205, 5.435221},
    {50.702149, 5.435617},
    {50.701901, 5.437354},
    {50.701822, 5.437976},
    {50.701737, 5.438626},
    {50.701633, 5.439406},
    {50.701595, 5.439725},
    {50.701595, 5.439725},
    {50.701545, 5.440146},
    {50.702055, 5.440323},
    {50.702743, 5.440577},
    {50.70313, 5.4407},
    {50.703209, 5.440732},
    {50.70332, 5.440738},
    {50.703429, 5.440732},
    {50.70353, 5.440687},
    {50.703692, 5.440598},
    {50.70389, 5.440487},
    {50.704014, 5.440429},
    {50.704014, 5.440429},
    {50.704238, 5.440323},
    {50.704373, 5.440239},
    {50.704363, 5.440417},
    {50.704335, 5.440695},
    {50.704281, 5.441036},
    {50.704064, 5.442083},
    {50.70397, 5.442648},
    {50.703867, 5.443347},
    {50.703809, 5.44374},
    {50.703788, 5.443874},
    {50.703758, 5.444057},
    {50.703664, 5.444524},
    {50.703571, 5.444834},
    {50.703525, 5.444937},
    {50.703464, 5.44507},
    {50.703399, 5.44518},
    {50.703352, 5.445253},
    {50.70328, 5.445349},
    {50.70309, 5.445596},
    {50.702676, 5.446111},
    {50.70254, 5.446272},
    {50.702538, 5.446275},
    {50.702538, 5.446275},
    {50.702401, 5.446431},
    {50.702335, 5.446519},
    {50.702176, 5.446673},
    {50.702241, 5.446837},
    {50.702424, 5.447338},
    {50.702585, 5.447781},
    {50.702786, 5.448347},
    {50.702941, 5.448845},
    {50.703022, 5.449096},
    {50.703083, 5.449249},
    {50.703315, 5.449693},
    {50.703385, 5.449852},
    {50.703445, 5.450022},
    {50.703469, 5.450144},
    {50.703486, 5.450236},
    {50.703495, 5.450405},
    {50.704141, 5.451526},
    {50.704245, 5.451701},
    {50.704245, 5.451701},
    {50.704646, 5.452374},
    {50.705226, 5.453528},
    {50.705713, 5.454597},
    {50.705739, 5.45464},
    {50.705772, 5.454676},
    {50.705827, 5.454715},
    {50.705865, 5.454735},
    {50.706252, 5.454874},
    {50.706726, 5.455044},
    {50.706846, 5.455098},
    {50.706876, 5.455115},
    {50.706919, 5.455143},
    {50.706924, 5.455148},
    {50.707076, 5.455239},
    {50.707194, 5.455335},
    {50.707194, 5.455335},
    {50.707484, 5.455573},
    {50.707644, 5.455702},
    {50.708213, 5.456132},
    {50.708544, 5.456397},
    {50.708826, 5.456597},
    {50.708981, 5.456729},
    {50.708998, 5.456743},
    {50.709138, 5.456888},
    {50.7093, 5.457084},
    {50.709567, 5.457437},
    {50.709591, 5.457468},
    {50.709791, 5.457744},
    {50.709898, 5.457926},
    {50.709964, 5.458082},
    {50.710062, 5.458351},
    {50.710153, 5.458637},
    {50.71032, 5.459218},
    {50.710415, 5.459496},
    {50.710495, 5.459719},
    {50.710568, 5.459923},
    {50.710648, 5.460102},
    {50.710859, 5.460625},
    {50.711155, 5.461365},
    {50.711155, 5.461365},
    {50.711465, 5.462143},
    {50.711613, 5.462309},
    {50.711765, 5.46248},
    {50.712101, 5.462915},
    {50.712526, 5.463405},
    {50.713021, 5.464013},
    {50.713356, 5.464468},
    {50.714282, 5.465406},
    {50.714392, 5.465548},
    {50.714332, 5.465847},
    {50.714015, 5.466096},
    {50.714015, 5.466096},
    {50.713311, 5.466648},
    {50.713118, 5.466779},
    {50.712559, 5.467157},
    {50.712276, 5.467317},
    {50.710914, 5.46809},
    {50.710766, 5.468191},
    {50.710766, 5.468191},
    {50.710339, 5.468483},
    {50.709955, 5.468826},
    {50.708288, 5.47045},
    {50.708192, 5.47053},
    {50.708192, 5.47053},
    {50.70708, 5.471468},
    {50.706987, 5.471518},
    {50.706946, 5.471541},
    {50.706901, 5.471567},
    {50.706854, 5.471598},
    {50.70653, 5.47182},
    {50.705549, 5.472494},
    {50.705284, 5.472676},
    {50.704978, 5.472887},
    {50.704869, 5.472968},
    {50.704708, 5.473123},
    {50.704593, 5.473246},
    {50.704382, 5.473499},
    {50.70364, 5.474615},
    {50.702725, 5.475993},
    {50.702716, 5.476007},
    {50.702477, 5.476443},
    {50.702253, 5.476852},
    {50.702253, 5.476852},
    {50.702237, 5.47688},
    {50.701942, 5.477383},
    {50.701682, 5.477745},
    {50.701733, 5.478036},
    {50.701784, 5.478287},
    {50.701847, 5.478563},
    {50.701909, 5.478828},
    {50.701964, 5.478997},
    {50.702025, 5.479173},
    {50.702087, 5.479303},
    {50.702134, 5.479395},
    {50.702215, 5.479541},
    {50.702317, 5.479704},
    {50.702472, 5.479738},
    {50.702593, 5.479762},
    {50.702775, 5.479829},
    {50.702835, 5.479857},
    {50.703072, 5.479964},
    {50.703778, 5.480375},
    {50.703778, 5.480375},
    {50.704235, 5.480642},
    {50.703723, 5.482036},
    {50.703334, 5.483157},
    {50.703091, 5.483954},
    {50.703044, 5.484107},
    {50.702981, 5.484313},
    {50.70286, 5.484686},
    {50.702726, 5.485099},
    {50.702525, 5.485563},
    {50.702382, 5.485874},
    {50.702382, 5.485874},
    {50.70236, 5.485921},
    {50.702052, 5.486555},
    {50.702092, 5.486967},
    {50.7021, 5.487298},
    {50.70209, 5.487625},
    {50.702073, 5.487891},
    {50.702193, 5.488021},
    {50.70225, 5.488083},
    {50.702301, 5.488145},
    {50.702345, 5.4882},
    {50.702388, 5.488259},
    {50.702439, 5.488337},
    {50.702528, 5.488444},
    {50.702425, 5.48864},
    {50.702425, 5.48864},
    {50.702096, 5.489267},
    {50.702033, 5.489403},
    {50.701986, 5.489531},
    {50.701952, 5.48966},
    {50.701921, 5.4898},
    {50.701892, 5.489975},
    {50.701888, 5.49014},
    {50.701888, 5.49053},
    {50.701892, 5.49085},
    {50.701867, 5.491054},
    {50.701845, 5.491217},
    {50.701814, 5.491384},
    {50.701762, 5.491556},
    {50.701693, 5.491738},
    {50.701623, 5.491905},
    {50.701602, 5.491951},
    {50.701548, 5.492069},
    {50.700918, 5.493254},
    {50.700918, 5.493254},
    {50.700708, 5.493651},
    {50.700162, 5.492872},
    {50.700135, 5.492834},
    {50.699527, 5.491835},
    {50.69945, 5.491692},
    {50.699414, 5.4916},
    {50.69937, 5.491469},
    {50.699324, 5.491314},
    {50.699278, 5.491096},
    {50.699251, 5.4909},
    {50.699222, 5.490676},
    {50.698579, 5.486156},
    {50.698579, 5.486156},
    {50.698494, 5.485555},
    {50.698365, 5.484741},
    {50.698308, 5.484472},
    {50.698302, 5.48445},
    {50.69822, 5.484116},
    {50.698091, 5.483668},
    {50.698007, 5.483393},
    {50.697865, 5.482904},
    {50.697837, 5.482796},
    {50.697814, 5.482706},
    {50.697766, 5.482495},
    {50.697766, 5.482495},
    {50.697628, 5.481903},
    {50.697558, 5.481967},
    {50.697551, 5.481977},
    {50.697546, 5.481989},
    {50.697523, 5.482109},
    {50.69747, 5.48232},
    {50.697407, 5.482529},
    {50.697353, 5.482698},
    {50.697286, 5.482883},
    {50.697232, 5.483007},
    {50.697169, 5.483135},
    {50.697122, 5.483219},
    {50.697043, 5.483329},
    {50.69695, 5.483415},
    {50.696717, 5.483571},
    {50.695872, 5.48398},
    {50.69341, 5.485127},
    {50.6932, 5.485275},
    {50.692976, 5.485451},
    {50.692206, 5.486176},
    {50.691417, 5.486958},
    {50.691211, 5.487132},
    {50.691011, 5.487271},
    {50.690841, 5.487389},
    {50.690663, 5.487494},
    {50.690258, 5.487684},
    {50.68799, 5.488594},
    {50.687539, 5.488824},
    {50.68731, 5.488985},
    {50.687069, 5.489224},
    {50.686715, 5.489624},
    {50.685738, 5.490813},
    {50.685585, 5.491002},
    {50.685585, 5.491002},
    {50.685515, 5.49109},
    {50.685258, 5.491406},
    {50.685114, 5.491584},
    {50.684838, 5.491983},
    {50.68464, 5.492286},
    {50.684383, 5.492694},
    {50.684266, 5.492881},
    {50.683973, 5.493362},
    {50.683739, 5.493885},
    {50.683405, 5.49463},
    {50.683343, 5.49477},
    {50.683199, 5.49509},
    {50.683046, 5.495433},
    {50.683046, 5.495433},
    {50.682614, 5.496399},
    {50.682375, 5.496902},
    {50.68211, 5.497459},
    {50.681978, 5.49776},
    {50.68174, 5.498305},
    {50.681615, 5.498613},
    {50.681526, 5.498843},
    {50.681399, 5.499164},
    {50.681364, 5.499248},
    {50.681269, 5.499468},
    {50.681131, 5.499882},
    {50.680953, 5.500547},
    {50.68085, 5.500946},
    {50.68069, 5.501567},
    {50.680594, 5.501955},
    {50.680594, 5.501955},
    {50.680559, 5.502096},
    {50.68038, 5.502806},
    {50.680153, 5.503692},
    {50.68, 5.504283},
    {50.679746, 5.505302},
    {50.679604, 5.50592},
    {50.679525, 5.506099},
    {50.679487, 5.506179},
    {50.679442, 5.506274},
    {50.679347, 5.506432},
    {50.679187, 5.506657},
    {50.679014, 5.506861},
    {50.67909, 5.507009},
    {50.679154, 5.507136},
    {50.679202, 5.507227},
    {50.679242, 5.507308},
    {50.679257, 5.507358},
    {50.679262, 5.507372},
    {50.679294, 5.507486},
    {50.679263, 5.507627},
    {50.67923, 5.507764},
    {50.679099, 5.508258},
    {50.678978, 5.508742},
    {50.678806, 5.5094},
    {50.678801, 5.509416},
    {50.678758, 5.509566},
    {50.678655, 5.509967},
    {50.678517, 5.510504},
    {50.678331, 5.511284},
    {50.678236, 5.511823},
    {50.678236, 5.511823},
    {50.67816, 5.512264},
    {50.677977, 5.513519},
    {50.677954, 5.513632},
    {50.677811, 5.514245},
    {50.677703, 5.514659},
    {50.677549, 5.515162},
    {50.677507, 5.515306},
    {50.677418, 5.515549},
    {50.677303, 5.515501},
    {50.676614, 5.515289},
    {50.676614, 5.515289},
    {50.676254, 5.515178},
    {50.67616, 5.515149},
    {50.67578, 5.515044},
    {50.675672, 5.515014},
    {50.675365, 5.514888},
    {50.675324, 5.514858},
    {50.675287, 5.514826},
    {50.675204, 5.514742},
    {50.675108, 5.514616},
    {50.674955, 5.514411},
    {50.674857, 5.51429},
    {50.674379, 5.513579},
    {50.674328, 5.513503},
    {50.674284, 5.513431},
    {50.674265, 5.513397},
    {50.674241, 5.513345},
    {50.674214, 5.513286},
    {50.674177, 5.513284},
    {50.674153, 5.513281},
    {50.674126, 5.513262},
    {50.674037, 5.513159},
    {50.673826, 5.512881},
    {50.673826, 5.512881},
    {50.673737, 5.512764},
    {50.673581, 5.512674},
    {50.673481, 5.512647},
    {50.673413, 5.512639},
    {50.673307, 5.512648},
    {50.673194, 5.512679},
    {50.673079, 5.51273},
    {50.672973, 5.512784},
    {50.672706, 5.512931},
    {50.672331, 5.513142},
    {50.671799, 5.51345},
    {50.671799, 5.51345},
    {50.671498, 5.513624},
    {50.671417, 5.512668},
    {50.671404, 5.512301},
    {50.671399, 5.511972},
    {50.671418, 5.511751},
    {50.671339, 5.511751},
    {50.671281, 5.51174},
    {50.67125, 5.511723},
    {50.671218, 5.511697},
    {50.671189, 5.511665},
    {50.671033, 5.511435},
    {50.670077, 5.510055},
    {50.670077, 5.510055},
    {50.669892, 5.509789},
    {50.669788, 5.509656},
    {50.66971, 5.509557},
    {50.669572, 5.509384},
    {50.669261, 5.508887},
    {50.669063, 5.508554},
    {50.669022, 5.508468},
    {50.668951, 5.508356},
    {50.668764, 5.508128},
    {50.66848, 5.507805},
    {50.668351, 5.507695},
    {50.668227, 5.507574},
    {50.667994, 5.507348},
    {50.667819, 5.50715},
    {50.667743, 5.507059},
    {50.667743, 5.507059},
    {50.66773, 5.507043},
    {50.667613, 5.506904},
    {50.667447, 5.506715},
    {50.667381, 5.506645},
    {50.667148, 5.50638},
    {50.667033, 5.506245},
    {50.666852, 5.506033},
    {50.666766, 5.505926},
    {50.666695, 5.50584},
    {50.666523, 5.505626},
    {50.666421, 5.505475},
    {50.666294, 5.50527},
    {50.666265, 5.505222},
    {50.666059, 5.505744},
    {50.665994, 5.505913},
    {50.665954, 5.506017},
    {50.665894, 5.506165},
    {50.665722, 5.506608},
    {50.665543, 5.507072},
    {50.665426, 5.507371},
    {50.665357, 5.507546},
    {50.665242, 5.507843},
    {50.665178, 5.508003},
    {50.665154, 5.508063},
    {50.665112, 5.508167},
    {50.66505, 5.508324},
    {50.664954, 5.508566},
    {50.664738, 5.509111},
    {50.664654, 5.509323},
    {50.664527, 5.509656},
    {50.664488, 5.509757},
    {50.664458, 5.509833},
    {50.664348, 5.510117},
    {50.66413, 5.510675},
    {50.66413, 5.510675},
    {50.664042, 5.510899},
    {50.663969, 5.511086},
    {50.663927, 5.511191},
    {50.663837, 5.511423},
    {50.663801, 5.511509},
    {50.663783, 5.511554},
    {50.663762, 5.511604},
    {50.663651, 5.511881},
    {50.663558, 5.512104},
    {50.663417, 5.512471},
    {50.663285, 5.512795},
    {50.663184, 5.51307},
    {50.663108, 5.513257},
    {50.663026, 5.513466},
    {50.662881, 5.513829},
    {50.662723, 5.514237},
    {50.662676, 5.514359},
    {50.662619, 5.514504},
    {50.662585, 5.514593},
    {50.662544, 5.514698},
    {50.662485, 5.514849},
    {50.662378, 5.515125},
    {50.662367, 5.515148},
    {50.662293, 5.515324},
    {50.662263, 5.515414},
    {50.662209, 5.515548},
    {50.662138, 5.515728},
    {50.662035, 5.515991},
    {50.661921, 5.516284},
    {50.66187, 5.516416},
    {50.661825, 5.516536},
    {50.661726, 5.516788},
    {50.661477, 5.517423},
    {50.661405, 5.517615},
    {50.661334, 5.5178},
    {50.661281, 5.517935},
    {50.661207, 5.518124},
    {50.661051, 5.518526},
    {50.660971, 5.518733},
    {50.660849, 5.519011},
    {50.660689, 5.519424},
    {50.660371, 5.520244},
    {50.660311, 5.520399},
    {50.65996, 5.521328},
    {50.65996, 5.521328},
    {50.659859, 5.521595},
    {50.65975, 5.521872},
    {50.659602, 5.52225},
    {50.659523, 5.522444},
    {50.659467, 5.522587},
    {50.659413, 5.522722},
    {50.659208, 5.523238},
    {50.659088, 5.523531},
    {50.658984, 5.523786},
    {50.658866, 5.524074},
    {50.658748, 5.524362},
    {50.658559, 5.52482},
    {50.658486, 5.525},
    {50.658416, 5.525168},
    {50.658292, 5.525455},
    {50.658186, 5.525702},
    {50.65786, 5.526472},
    {50.657748, 5.526724},
    {50.657391, 5.527524},
    {50.656922, 5.528589},
    {50.656683, 5.529118},
    {50.656681, 5.52919},
    {50.656512, 5.529559},
    {50.656119, 5.530444},
    {50.655969, 5.530792},
    {50.655969, 5.530792},
    {50.65587, 5.531024},
    {50.655702, 5.531375},
    {50.655057, 5.532936},
    {50.654959, 5.533163},
    {50.654333, 5.535053},
    {50.654286, 5.535197},
    {50.654163, 5.535514},
    {50.654002, 5.535986},
    {50.653956, 5.53611},
    {50.653613, 5.537084},
    {50.652686, 5.539765},
    {50.652645, 5.539894},
    {50.652582, 5.540095},
    {50.65253, 5.540227},
    {50.65253, 5.540227},
    {50.652347, 5.540697},
    {50.651895, 5.541989},
    {50.65135, 5.543604},
    {50.650967, 5.54474},
    {50.650583, 5.545946},
    {50.650502, 5.546143},
    {50.650261, 5.546766},
    {50.650193, 5.546955},
    {50.650109, 5.547201},
    {50.649982, 5.547639},
    {50.649961, 5.547744},
    {50.649938, 5.547837},
    {50.649903, 5.547937},
    {50.649876, 5.548022},
    {50.649806, 5.548193},
    {50.649702, 5.548343},
    {50.649702, 5.548343},
    {50.649614, 5.548471},
    {50.649493, 5.548648},
    {50.649324, 5.548858},
    {50.648471, 5.549855},
    {50.648381, 5.549956},
    {50.648354, 5.549988},
    {50.64832, 5.550035},
    {50.648285, 5.550098},
    {50.648236, 5.550183},
    {50.648211, 5.550235},
    {50.64819, 5.550291},
    {50.64815, 5.55039},
    {50.648105, 5.550574},
    {50.648071, 5.550718},
    {50.648006, 5.551032},
    {50.6479, 5.551753},
    {50.647754, 5.552605},
    {50.647696, 5.552958},
    {50.647575, 5.553626},
    {50.647507, 5.553942},
    {50.647495, 5.554014},
    {50.647495, 5.554014},
    {50.647378, 5.554759},
    {50.647207, 5.555749},
    {50.647132, 5.556086},
    {50.647085, 5.556268},
    {50.647006, 5.556524},
    {50.646917, 5.556781},
    {50.646929, 5.556839},
    {50.646953, 5.556918},
    {50.647057, 5.557196},
    {50.647137, 5.557392},
    {50.64719, 5.557485},
    {50.647221, 5.557546},
    {50.647246, 5.557669},
    {50.647265, 5.557763},
    {50.647272, 5.557831},
    {50.647276, 5.55792},
    {50.64716, 5.558053},
    {50.647009, 5.558221},
    {50.647004, 5.558228},
    {50.646914, 5.558349},
    {50.646872, 5.558419},
    {50.646835, 5.558481},
    {50.64682, 5.558528},
    {50.64681, 5.558557},
    {50.64681, 5.558557},
    {50.646796, 5.558601},
    {50.646772, 5.558719},
    {50.646744, 5.558852},
    {50.646741, 5.558957},
    {50.646719, 5.559523},
    {50.646696, 5.559758},
    {50.646717, 5.560585},
    {50.646722, 5.560929},
    {50.646731, 5.561112},
    {50.646766, 5.561623},
    {50.646812, 5.562063},
    {50.646837, 5.56221},
    {50.646899, 5.562486},
    {50.646917, 5.562562},
    {50.646952, 5.562708},
    {50.647019, 5.563014},
    {50.64719, 5.563723},
    {50.647203, 5.563765},
    {50.647245, 5.563908},
    {50.647262, 5.564021},
    {50.647268, 5.564072},
    {50.647272, 5.564123},
    {50.647278, 5.564182},
    {50.64729, 5.564303},
    {50.647297, 5.564364},
    {50.647302, 5.564439},
    {50.647303, 5.564546},
    {50.6473, 5.564667},
    {50.647271, 5.564997},
    {50.647224, 5.565432},
    {50.647224, 5.565432},
    {50.647189, 5.565749},
    {50.647166, 5.565932},
    {50.647043, 5.566903},
    {50.647018, 5.567064},
    {50.64697, 5.567232},
    {50.646913, 5.567394},
    {50.646769, 5.5677},
    {50.646605, 5.568074},
    {50.646534, 5.568234},
    {50.646456, 5.56838},
    {50.646366, 5.568478},
    {50.64629, 5.568529},
    {50.646135, 5.568637},
    {50.646103, 5.568667},
    {50.646072, 5.568727},
    {50.646, 5.568966},
    {50.645994, 5.568984},
    {50.645979, 5.569027},
    {50.64596, 5.569086},
    {50.64596, 5.569086},
    {50.645916, 5.569222},
    {50.64588, 5.569326},
    {50.645795, 5.5695},
    {50.645777, 5.569575},
    {50.645758, 5.569656},
    {50.645757, 5.56975},
    {50.645788, 5.569857},
    {50.645859, 5.570113},
    {50.645992, 5.570512},
    {50.646078, 5.570756},
    {50.646123, 5.570893},
    {50.646148, 5.570993},
    {50.646164, 5.571098},
    {50.646183, 5.571278},
    {50.64619, 5.571435},
    {50.646189, 5.571736},
    {50.646176, 5.571935},
    {50.646142, 5.572098},
    {50.646112, 5.57217},
    {50.646054, 5.57226},
    {50.645925, 5.572386},
    {50.645775, 5.572522},
    {50.645707, 5.572611},
    {50.645698, 5.572627},
    {50.645684, 5.572647},
    {50.645656, 5.572681},
    {50.645594, 5.572717},
    {50.645455, 5.572829},
    {50.645401, 5.572856},
    {50.645319, 5.572919},
    {50.644902, 5.573222},
    {50.644887, 5.573201},
    {50.644848, 5.57317},
    {50.644811, 5.573163},
    {50.644774, 5.573189},
    {50.644745, 5.573225},
    {50.644733, 5.573296},
    {50.644733, 5.573362},
    {50.644741, 5.57341},
    {50.644756, 5.573439},
    {50.644767, 5.573453},
    {50.644778, 5.573464},
    {50.64481, 5.573484},
    {50.644837, 5.573486},
    {50.644877, 5.573459},
    {50.644887, 5.573445},
    {50.644905, 5.573412},
    {50.644916, 5.57336},
    {50.644918, 5.573337},
    {50.64492, 5.573302},
    {50.644916, 5.573261},
    {50.644913, 5.573242},
    {50.644902, 5.573222},
    {50.645236, 5.572979}};

class FakeGPS
{
public:
    FakeGPS()
    {
        std::thread t(&FakeGPS::run, this);
        t.detach();
    }

    const double *getCoords() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return coords;
    }

    void setCoords(double lat, double lon)
    {
        std::unique_lock<std::shared_mutex> lock(mutex);
        coords[0] = lat;
        coords[1] = lon;
    }

    void run()
    {
        while (true)
        {
            for (size_t i = 0; i < FAKE_LENGTH; i++)
            {
                setCoords(POSITION[i][0], POSITION[i][1]);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }

            for (size_t i = FAKE_LENGTH - 1; i > 0; i--)
            {
                setCoords(POSITION[i][0], POSITION[i][1]);
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }

private:
    double coords[2] = {0, 0};
    mutable std::shared_mutex mutex;
};
void handle_client(int client_fd, FakeGPS *gps);

void handle_client(int client_fd, FakeGPS *gps)
{
    // Loop to continuously send values to this particular client
    while (true)
    {
        const double *values = gps->getCoords();
        // Some times send fails (and indeed crash whole main process when connection is closed),
        // so we check if the client is still connected
        struct pollfd fds;
        fds.fd = client_fd;
        fds.events = POLLIN;
        int ret = poll(&fds, 1, 1000);
        if (ret > 0)
        {
            std::cout << "Client disconnected or error occurred" << std::endl;
            break;
        }

        // Send the values
        ssize_t sent_bytes = send(client_fd, values, sizeof(double) * 2, 0);
        if (sent_bytes != sizeof(double) * 2)
        {
            std::cout << "Error sending data" << std::endl;
            break;
        }

        sleep(1);
    }
    close(client_fd);
}

int main()
{
    int server_fd, client_fd;
    struct sockaddr_un address;
    int addrlen = sizeof(address);

    FakeGPS *gps = new FakeGPS();

    // make socket
    if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Connectong
    unlink(SOCKET_PATH); // in case it already exists
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, SOCKET_PATH, sizeof(address.sun_path) - 1);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Emulator listening on " << SOCKET_PATH << std::endl;
    std::vector<std::shared_ptr<std::thread>> threads;

    while (true)
    {

        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            continue; // Continue accepting other clients even if one accept fails
        }

        // Spawn a new thread for each connected client
        threads.push_back(std::make_shared<std::thread>(handle_client, client_fd, gps));
    }

    // this should never be reached since the server is always running
    for (std::shared_ptr<std::thread> t : threads)
    {
        t->join();
    }

    close(server_fd);
    unlink(SOCKET_PATH);
    delete gps;
    return 0;
}
