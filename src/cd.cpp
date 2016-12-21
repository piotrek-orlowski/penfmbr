// Runs (very straightforward) coordinate descent on the objective
//
// 1\2n||y - w0 - Xw||^2_2 + P(alpha, x, w)
//
// where
//
// P = alpha *  sum_{i=1}^{p} |w_i| * alpha_weights[i]
//
// i.e. plain Lasso where each l1 penalty coordinate is weighted according to
// `alpha_weights[i]`
//
// Reference:
// Hastie, T., Tibshirani, R., & Wainwright, M. (2015). Statistical learning
// with sparsity: the lasso and generalizations. CRC Press.
//
// [[Rcpp::depends(RcppArmadillo)]]
#include <RcppArmadillo.h>

double soft_thresh(double x, double value) {
  int sign = x < 0 ? -1 : 1;
  return sign * std::max(std::abs(x) - value, 0.0);
}

//' @export
// [[Rcpp::export]]
Rcpp::List cd(arma::mat X, arma::vec y, double alpha,
              arma::vec alpha_weights, double tol, int maxiter) {

  int n = X.n_rows;
  int p = X.n_cols;
  bool converged;
  double ybar = arma::mean(y);
  arma::mat Xbar = arma::mean(X, 0);
  arma::mat Xstd = arma::stddev(X, 0);
  arma::vec r;
  arma::vec w = arma::zeros(p);
  arma::vec wp = arma::zeros(p);

  // Fit on centered y and normalized X
  y = y - ybar;
  X = (X - arma::repmat(Xbar, n, 1)) / arma::repmat(Xstd, n, 1);

  for (int n_iter = 0; n_iter < maxiter; n_iter++) {
    r = y - X * w;

    for (int i = 0; i < p; i++)
      w(i) = soft_thresh(w(i) + dot(X.col(i), r) / n, alpha * alpha_weights(i));

    converged =  arma::norm(w - wp) < tol;
    if (converged)
      break;

    wp = w;
  }

  if (!converged)
    Rcpp::warning("Coordinate descent did not converge. You might want to increase `maxiter` or decrease `tol`");

  // Return coefficients on original scale
  w /= Xstd.t();
  double intercept = ybar - dot(Xbar, w);

  return Rcpp::List::create(
    Rcpp::Named("w0") = intercept, Rcpp::Named("w") = w);
}