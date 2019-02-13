import numpy as np

from pymor.basic import *
from pymor.bindings.dunext import DuneXTMatrixOperator, DuneXTVectorSpace
from pymor.bindings.dunegdt import DuneGDTVisualizer
from pymor.vectorarrays.list import ListVectorArray

from dune.xt.la import IstlDenseVectorDouble
from dune.xt.functions import ConstantFunction__2d_to_1x1 as ConstantFunction
from dune.xt.functions import ExpressionFunction__2d_to_1x1 as ExpressionFunction
# from dune.xt.functions import IndicatorFunction__2d_to_1x1 as IndicatorFunction
from dune.xt.functions import IndicatorGridFunction__2d_simplex_aluconformgrid_to_1x1 as IndicatorFunction
from dune.gdt.gamm_2019_talk_on_conservative_rb import function_to_grid_function, Spe10Model1Function

logger = getLogger('main.main');
set_log_levels({'main': 'INFO',
                'pymor.discretizations': 'WARN'})

channel = IndicatorFunction([
            ([[1.70, 1.75], [0.50, 0.55]], [-1.077632394950]),
            ([[1.75, 1.80], [0.50, 0.55]], [-1.076995127720]),
            ([[1.80, 1.85], [0.50, 0.55]], [-1.073561564390]),
            ([[1.85, 1.90], [0.50, 0.55]], [-1.066022817360]),
            ([[1.90, 1.95], [0.50, 0.55]], [-1.065036837430]),
            ([[1.95, 2.00], [0.50, 0.55]], [-1.079748704260]),
            ([[2.00, 2.05], [0.50, 0.55]], [-1.056658959230]),
            ([[2.05, 2.10], [0.50, 0.55]], [-1.083103348370]),
            ([[2.10, 2.15], [0.50, 0.55]], [-1.058654849730]),
            ([[2.15, 2.20], [0.50, 0.55]], [-1.058710395350]),
            ([[2.20, 2.25], [0.50, 0.55]], [-1.081366959010]),
            ([[2.25, 2.30], [0.50, 0.55]], [-1.084901727210]),
            ([[2.30, 2.35], [0.50, 0.55]], [-1.066411207580]),
            ([[2.35, 2.40], [0.50, 0.55]], [-1.068127732980]),
            ([[2.40, 2.45], [0.50, 0.55]], [-1.076956520490]),
            ([[2.45, 2.50], [0.50, 0.55]], [-1.086300792050]),
            ([[2.50, 2.55], [0.50, 0.55]], [-1.082737221120]),
            ([[2.55, 2.60], [0.50, 0.55]], [-1.075004021550]),
            ([[2.60, 2.65], [0.50, 0.55]], [-1.086071425620]),
            ([[2.65, 2.70], [0.50, 0.55]], [-1.072687617990]),
            ([[2.70, 2.75], [0.50, 0.55]], [-1.085370373620]),
            ([[2.75, 2.80], [0.50, 0.55]], [-1.084669272730]),
            ([[2.80, 2.85], [0.50, 0.55]], [-1.084446618150]),
            ([[2.85, 2.90], [0.50, 0.55]], [-1.089570379670]),
            ([[2.90, 2.95], [0.50, 0.55]], [-1.080473940520]),
            ([[2.95, 3.00], [0.50, 0.55]], [-1.082212290830]),
            ([[3.00, 3.05], [0.50, 0.55]], [-1.085685998630]),
            ([[3.05, 3.10], [0.50, 0.55]], [-1.084283478720]),
            ([[3.10, 3.15], [0.50, 0.55]], [-1.091040987340]),
            ([[3.15, 3.20], [0.50, 0.55]], [-1.094927006730]),
            ([[3.20, 3.25], [0.50, 0.55]], [-1.097604405370]),
            ([[3.25, 3.30], [0.50, 0.55]], [-1.096449894530]),
            ([[3.30, 3.35], [0.50, 0.55]], [-1.094416810250]),
            ([[3.35, 3.40], [0.50, 0.55]], [-1.095332906540]),
            ([[3.40, 3.45], [0.50, 0.55]], [-1.100143080800]),
            ([[3.45, 3.50], [0.50, 0.55]], [-1.100656276210]),
            ([[3.50, 3.55], [0.50, 0.55]], [-1.101258771860]),
            ([[3.55, 3.60], [0.50, 0.55]], [-1.100574858930]),
            ([[3.60, 3.65], [0.50, 0.55]], [-1.100022619060]),
            ([[3.65, 3.70], [0.50, 0.55]], [-1.102191542090]),
            ([[3.70, 3.75], [0.50, 0.55]], [-1.099944638010]),
            ([[3.75, 3.80], [0.50, 0.55]], [-1.102656305330]),
            ([[3.80, 3.85], [0.50, 0.55]], [-1.104485665260]),
            ([[3.85, 3.90], [0.50, 0.55]], [-1.107358201210]),
            ([[3.90, 3.95], [0.50, 0.55]], [-1.107002236700]),
            ([[3.95, 4.00], [0.50, 0.55]], [-1.107776503870]),
            ([[4.00, 4.05], [0.50, 0.55]], [-1.108927855620]),
            ([[2.60, 2.65], [0.45, 0.50]], [-1.103725892110]),
            ([[2.65, 2.70], [0.45, 0.50]], [-1.102088998800]),
            ([[2.70, 2.75], [0.45, 0.50]], [-1.098069550690]),
            ([[2.75, 2.80], [0.45, 0.50]], [-1.100009024210]),
            ([[2.80, 2.85], [0.45, 0.50]], [-1.087974687240]),
            ([[2.85, 2.90], [0.45, 0.50]], [-1.088274721760]),
            ([[2.90, 2.95], [0.45, 0.50]], [-1.086922371090]),
            ([[2.95, 3.00], [0.45, 0.50]], [-1.078931900930]),
            ([[3.00, 3.05], [0.45, 0.50]], [-1.087483738530]),
            ([[3.05, 3.10], [0.45, 0.50]], [-1.074451973240]),
            ([[3.10, 3.15], [0.45, 0.50]], [-1.082466131630]),
            ([[3.15, 3.20], [0.45, 0.50]], [-1.067267905040]),
            ([[3.20, 3.25], [0.45, 0.50]], [-1.078912178470]),
            ([[3.25, 3.30], [0.45, 0.50]], [-1.072608271260]),
            ([[3.30, 3.35], [0.45, 0.50]], [-1.070940627480]),
            ([[3.35, 3.40], [0.45, 0.50]], [-1.069239942900]),
            ([[3.40, 3.45], [0.45, 0.50]], [-1.000998857010]),
            ([[3.45, 3.50], [0.45, 0.50]], [-1.001095440020]),
            ([[3.50, 3.55], [0.45, 0.50]], [-0.966491003242]),
            ([[3.55, 3.60], [0.45, 0.50]], [-0.802284684014]),
            ([[3.60, 3.65], [0.45, 0.50]], [-0.980790923021]),
            ([[3.65, 3.70], [0.45, 0.50]], [-0.614478271687]),
            ([[3.70, 3.75], [0.45, 0.50]], [-0.288129858959]),
            ([[3.75, 3.80], [0.45, 0.50]], [-0.929509396842]),
            ([[3.80, 3.85], [0.45, 0.50]], [-0.992376505995]),
            ([[3.85, 3.90], [0.45, 0.50]], [-0.968162494855]),
            ([[3.90, 3.95], [0.45, 0.50]], [-0.397316938901]),
            ([[3.95, 4.00], [0.45, 0.50]], [-0.970934956609]),
            ([[4.00, 4.05], [0.45, 0.50]], [-0.784344730096]),
            ([[4.05, 4.10], [0.45, 0.50]], [-0.539725422323]),
            ([[4.10, 4.15], [0.45, 0.50]], [-0.915632282372]),
            ([[4.15, 4.20], [0.45, 0.50]], [-0.275089177273]),
            ([[4.20, 4.25], [0.45, 0.50]], [-0.949684959286]),
            ([[4.25, 4.30], [0.45, 0.50]], [-0.936132529794]),
            ([[1.95, 2.00], [0.40, 0.45]], [-1.109236427950]),
            ([[2.00, 2.05], [0.40, 0.45]], [-1.106856186230]),
            ([[2.05, 2.10], [0.40, 0.45]], [-1.105780037600]),
            ([[2.10, 2.15], [0.40, 0.45]], [-1.101877236290]),
            ([[2.15, 2.20], [0.40, 0.45]], [-1.103517104640]),
            ([[2.20, 2.25], [0.40, 0.45]], [-1.100375511370]),
            ([[2.25, 2.30], [0.40, 0.45]], [-1.097244070760]),
            ([[2.30, 2.35], [0.40, 0.45]], [-1.096046002080]),
            ([[2.35, 2.40], [0.40, 0.45]], [-1.093544696560]),
            ([[2.40, 2.45], [0.40, 0.45]], [-1.089344553540]),
            ([[2.45, 2.50], [0.40, 0.45]], [-1.081554765860]),
            ([[2.50, 2.55], [0.40, 0.45]], [-1.078153978990]),
            ([[2.55, 2.60], [0.40, 0.45]], [-1.091740620230]),
            ([[2.60, 2.65], [0.40, 0.45]], [-1.074336160680]),
            ([[2.65, 2.70], [0.40, 0.45]], [-1.080305877010]),
            ([[2.25, 2.30], [0.35, 0.40]], [-1.000328694070]),
            ([[2.30, 2.35], [0.35, 0.40]], [-1.011759089050]),
            ([[2.35, 2.40], [0.35, 0.40]], [-1.049543957930]),
            ([[2.40, 2.45], [0.35, 0.40]], [-1.017967697000]),
            ([[2.45, 2.50], [0.35, 0.40]], [-1.046471840910]),
            ([[2.50, 2.55], [0.35, 0.40]], [-1.019118948310]),
            ([[2.55, 2.60], [0.35, 0.40]], [-1.006993401580]),
            ([[2.60, 2.65], [0.35, 0.40]], [-0.995492960025]),
            ([[2.65, 2.70], [0.35, 0.40]], [-1.037305900700])
        ],
        'channel')
diffusion_factor = {'functions': (function_to_grid_function(ConstantFunction(1)) + channel,
                                  function_to_grid_function(ConstantFunction(-1))*channel),
                    'coefficients': (ExpressionParameterFunctional('1', {'switch': ()}),
                                     ProjectionParameterFunctional('switch', ()))}

def make_diffusion_factor_mu(mu):
    return (function_to_grid_function(ConstantFunction(1))
            + function_to_grid_function(ConstantFunction(1 - mu['switch']))*channel)

mu_bar = {'switch': 0.1}
mu_hat = {'switch': 0.1}

diffusion_factor_bar = make_diffusion_factor_mu(mu_bar)
diffusion_factor_hat = make_diffusion_factor_mu(mu_hat)
diffusion_factor_min = make_diffusion_factor_mu({'switch': 0.1})
diffusion_factor_max = make_diffusion_factor_mu({'switch': 1.0})

diffusion_tensor = Spe10Model1Function([0, 0], [5, 1])

f = IndicatorFunction([([[0.95, 1.10], [0.30, 0.45]], [2e3]),
                       ([[3.00, 3.15], [0.75, 0.90]], [-1e3]),
                       ([[4.25, 4.40], [0.25, 0.40]], [-1e3])],
                       'force')
zero = ConstantFunction(0.)

def alpha(mu, mu_bar):
    return np.min([theta.evaluate(mu) / theta.evaluate(mu_bar) for theta in diffusion_factor['coefficients']])

def gamma(mu, mu_bar):
    return np.max([theta.evaluate(mu) / theta.evaluate(mu_bar) for theta in diffusion_factor['coefficients']])

from dune.gdt.gamm_2019_talk_on_conservative_rb import (
        DiscontinuousLagrangeSpace,
        GridProvider,
        RaviartThomasSpace,
        assemble_energy_semi_product_matrix,
        assemble_DG_product_matrix,
        assemble_Hdiv_product_matrix,
        assemble_L2_vector,
        assemble_SWIPDG_matrix,
        compute_estimate,
        compute_flux_reconstruction,
        compute_local_conservation_error,
        make_discrete_function,
        prolong,
        visualize,
        )

def make_marix_operator(mat, ID):
    return DuneXTMatrixOperator(mat, source_id=ID, range_id=ID)

logger.info('discretizing ...')

def discretize(num_refinements):
    grid = GridProvider([0, 0], [5, 1], [100, 20]) # The OS2015 test is [0, 5]x[0, 1], 100x20 elements, ...
    grid.refine(num_refinements)
    dg_space = DiscontinuousLagrangeSpace(grid, 1)

    lhs_op = LincombOperator([make_marix_operator(assemble_SWIPDG_matrix(dg_space, diff, diffusion_tensor), 'PRESSURE')
                              for diff in diffusion_factor['functions']],
                             diffusion_factor['coefficients'])
    rhs_func = VectorFunctional(lhs_op.range.make_array((assemble_L2_vector(dg_space, f),)))
    dg_product = make_marix_operator(assemble_DG_product_matrix(dg_space, diffusion_factor_bar, diffusion_tensor), 'PRESSURE')

    fom = StationaryDiscretization(lhs_op, rhs_func, products={'h1_penalty': dg_product},
                                   visualizer=DuneGDTVisualizer(dg_space))
    fom = fom.with_(parameter_space=CubicParameterSpace(fom.parameter_type, 0.1, 1.))
    return grid, dg_space, dg_product, fom

grid, dg_space, dg_product, fom = discretize(2) # ... and 2 refinements with ALU_2D_SIMPLEX_CONFORMING
PressureVectorSpace = DuneXTVectorSpace(IstlDenseVectorDouble, dg_space.num_DoFs, 'PRESSURE')

logger.info('grid has {} elements'.format(grid.num_elements))
logger.info('space has {} DoFs'.format(dg_space.num_DoFs))
logger.info('')

# visualize(grid, make_diffusion_factor_min*diffusion_tensor, 'diffusion_min')
# visualize(grid, make_diffusion_factor_max*diffusion_tensor, 'diffusion_max')
# visualize(grid, f, 'force')

logger.info('computing reference discretization ...')

reference_grid, reference_dg_space, _, reference_fom = discretize(2 + 2*2)
reference_energy_semi_product = make_marix_operator(
        assemble_energy_semi_product_matrix(reference_dg_space, diffusion_factor_bar, diffusion_tensor), 'PRESSURE')
ReferencePressureVectorSpace = DuneXTVectorSpace(IstlDenseVectorDouble, reference_dg_space.num_DoFs, 'PRESSURE')

def reference_dg_norm(u):
    if not isinstance(u, ListVectorArray):
        u = ReferencePressureVectorSpace.from_data([u,])
    return np.sqrt(reference_energy_semi_product.apply2(u, u)[0][0])

logger.info('')
logger.info('assembling Hdiv product ...')

rtn_space = RaviartThomasSpace(grid, 0)
FluxVectorSpace = DuneXTVectorSpace(IstlDenseVectorDouble, rtn_space.num_DoFs, 'FLUX')
rtn_product = make_marix_operator(assemble_Hdiv_product_matrix(rtn_space), 'FLUX')

def rtn_norm(t):
    if not isinstance(t, ListVectorArray):
        t = FluxVectorSpace.from_data([t,])
    return np.sqrt(rtn_product.apply2(t, t)[0][0])

logger.info('')
logger.info('computing affine shift pressure and flux ...')

u_h_f = make_marix_operator(assemble_SWIPDG_matrix(dg_space, diffusion_factor_bar, diffusion_tensor),
                            'PRESSURE').apply_inverse(fom.rhs.as_source_array())
t_h_f = FluxVectorSpace.from_data([
    compute_flux_reconstruction(grid, dg_space, rtn_space, diffusion_factor_bar, diffusion_tensor, u_h_f._list[0].impl),])

logger.info('')
logger.info('computing [OS2015, table 4] estimates (should be 2.13, 1.88e-09, 0.966) ...')

u_h = make_marix_operator(assemble_SWIPDG_matrix(dg_space, diffusion_factor_max, diffusion_tensor),
                            'PRESSURE').apply_inverse(fom.rhs.as_source_array())
t_h = FluxVectorSpace.from_data([
    compute_flux_reconstruction(grid, dg_space, rtn_space, diffusion_factor_max, diffusion_tensor, u_h._list[0].impl),])
_, eta_NC, eta_R, eta_DF = compute_estimate(
        grid,
        make_discrete_function(dg_space, u_h._list[0].impl, 'u_h'),
        make_discrete_function(rtn_space, t_h._list[0].impl, 't_h'),
        f, diffusion_factor_max, diffusion_factor_max, diffusion_factor_max, diffusion_tensor,
        1, 1, 1)
del u_h
del t_h

logger.info('    are {}, {}, {}'.format(eta_NC, eta_R, eta_DF))
logger.info('')
max_extensions=1
logger.info('building pressure RB (simulating intermediate greedy step by max_extensions={}) ...'.format(
    max_extensions))
logger.info('')

reductor = CoerciveRBReductor(fom,
        product=fom.h1_penalty_product,
        coercivity_estimator=ExpressionParameterFunctional('switch', fom.parameter_type))
training_set = fom.parameter_space.sample_uniformly(100)
greedy_data = greedy(fom, reductor, training_set,
        extension_params={'method': 'gram_schmidt'}, max_extensions=max_extensions)
rom = greedy_data['rd']
greedy_mus = greedy_data['max_err_mus']

logger.info('')
logger.info('building Hdiv_0 RB ...')

Hdiv_0_RB = FluxVectorSpace.empty()
for ii in range(len(greedy_data['max_err_mus'])):
    mu = greedy_data['max_err_mus'][ii]
    u_h = fom.solve(mu)
    diffusion_factor_mu = make_diffusion_factor_mu(mu)
    t_h = FluxVectorSpace.from_data([
        compute_flux_reconstruction(grid, dg_space, rtn_space, diffusion_factor_mu, diffusion_tensor, u_h._list[0].impl),])
    Hdiv_0_RB.append(t_h - t_h_f)
Hdiv_0_RB = gram_schmidt(Hdiv_0_RB, product=rtn_product, copy=False)

logger.info('')
logger.info('testing for some parameters ...')

for mu in fom.parameter_space.sample_uniformly(3):
    logger.info('  mu = {}'.format(mu))
    u_RB = rom.solve(mu)
    u_RB = reductor.reconstruct(u_RB)
    diffusion_factor_mu = make_diffusion_factor_mu(mu)
    t_RB = FluxVectorSpace.from_data([
        compute_flux_reconstruction(grid, dg_space, rtn_space, diffusion_factor_mu, diffusion_tensor, u_RB._list[0].impl),])
    # logger.info('    conservation_error(t_RB): {}'.format(compute_local_conservation_error(
        # grid, make_discrete_function(rtn_space, t_RB._list[0].impl, 't_RB'), f)))
    # project onto Hdiv_0_RB (we know that Hdiv_0_R is orthonormalized wrt rtn_product)
    t_RB_0 = rtn_product.apply2(Hdiv_0_RB, FluxVectorSpace.from_data([t_RB._list[0].impl - t_h_f._list[0].impl,]))
    t_RB_0 = Hdiv_0_RB.lincomb(t_RB_0[:,0])
    t_RB_f = t_RB_0._list[0].impl + t_h_f._list[0].impl
    # logger.info('    conservation_error(t_RB_f): {}'.format(compute_local_conservation_error(
        # grid, make_discrete_function(rtn_space, t_RB_f, 't_RB_f'), f)))
    # compare to correct flux
    # u_h = fom.solve(mu)._list[0].impl
    # t_h = compute_flux_reconstruction(grid, dg_space, rtn_space, diffusion_factor_mu, diffusion_tensor, u_h)
    # logger.info('    relative flux reconstruction error: {}'.format(rtn_norm(t_h - t_RB_f) / rtn_norm(t_h)))
    # compute estimate
    logger.info('    estimating error ...')
    eta, eta_NC, eta_R, eta_DF = compute_estimate(
            grid,
            make_discrete_function(dg_space, u_RB._list[0].impl, 'u_RB'),
            make_discrete_function(rtn_space, t_RB_f, 't_RB_f'),
            f, diffusion_factor_mu, diffusion_factor_bar, diffusion_factor_hat, diffusion_tensor,
            alpha(mu, mu_bar), alpha(mu, mu_hat), gamma(mu, mu_bar))
    logger.info('      eta_NC = {}'.format(eta_NC))
    logger.info('      eta_R  = {}'.format(eta_R))
    logger.info('      eta_DF = {}'.format(eta_DF))
    logger.info('      eta    = {}'.format(eta))
    logger.info('    computing error by reference solution ...')
    u_RB_on_ref = prolong(dg_space, u_RB._list[0].impl, reference_dg_space)
    u_h_ref = reference_fom.solve(mu)
    error = reference_dg_norm(u_h_ref._list[0].impl - u_RB_on_ref)
    logger.info('      error  = {}'.format(error))
    logger.info('   => efficiency = {}'.format(eta/error))

