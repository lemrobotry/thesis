#ifndef _PERMUTE_SGD_HH
#define _PERMUTE_SGD_HH

namespace Permute {

  // With maximization as the objective, increases the current parameter values
  // by their gradients, multiplied by a learning rate.  If the gradient is
  // positive, increasing the parameter value will increase the objective,
  // whereas if the gradient is negative, decreasing the parameter value will
  // increase the objective.
  class UpdateSGD : public std::binary_function <double, double, double> {
  private:
    double learning_rate_;
    double time_;
  public:
    UpdateSGD (double learning_rate) :
      learning_rate_ (learning_rate),
      time_ (1.0)
    {}
    double operator () (double value, double gradient) const {
      return value + learningRate () * gradient;
    }
    double learningRate () const {
      return learning_rate_;
    }
    void tick () {
      learning_rate_ *= time_;
      learning_rate_ /= ++ time_;
    }
  };

}

#endif//_PERMUTE_SGD_HH
